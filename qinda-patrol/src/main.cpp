#include "patrol_window.hpp"
#ifdef PATROL_HAVE_LAYER_SHELL
#include <LayerShellQt/Window>
#endif
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QRandomGenerator>
#include <QScreen>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <vector>

namespace {
void describeScreens(const QList<QScreen*>& screens){
    for(int i=0;i<screens.size();++i){const auto* s=screens[i];const QRect g=s->geometry();
        std::cout<<i<<": "<<s->name().toStdString()<<"  "<<g.width()<<'x'<<g.height()<<" at "<<g.x()<<','<<g.y()<<"  scale "<<s->devicePixelRatio()<<"  "<<s->refreshRate()<<" Hz"
                 <<(s==QGuiApplication::primaryScreen()?"  primary":"")<<(s->model().isEmpty()?"":"  "+s->model().toStdString())<<'\n';}
}
}
int main(int argc,char** argv){
    QGuiApplication app(argc,argv);
    QGuiApplication::setApplicationName("qinda-patrol");QGuiApplication::setApplicationVersion("0.2.0");QGuiApplication::setOrganizationName("QindaQt");QGuiApplication::setDesktopFileName("org.qindaqt.Patrol");
    QCommandLineParser parser;parser.setApplicationDescription("An autonomous, read-only cyberpunk platformer screensaver. Not a session locker.");parser.addHelpOption();parser.addVersionOption();
    parser.addOptions({
        {{"p","preview"},"Run in a normal resizable window (default)."},
        {"screensaver","Fullscreen on every connected output; exit on input. Does not lock the session."},
        {"all-screens","Accepted for compatibility; --screensaver already covers every output."},
        {"screen","Use only this output index (see --list-screens). Preview defaults to the primary output.","index"},
        {"list-screens","List connected outputs with their index and exit."},
        {"no-layer-shell","Use plain fullscreen windows instead of Wayland layer-shell surfaces."},
        {"verbose","Log window placement and exposure to stderr."},
        {"fps","Presentation rate, 1–60. Simulation remains fixed-step.","fps","30"},
        {"eco","Cap presentation at 20 frames per second."},
        {"seed","Reproducible unsigned 64-bit world seed; decimal or 0x hex.","seed"},
        {"demo","Clearly labeled synthetic metrics; never reads /proc."},
        {"no-metrics","Replace metric signs with artwork; never reads /proc."},
        {"interface","Use this network interface instead of the IPv4 default route.","name"},
        {"no-overlay","Hide the small drifting title and patch counter."},
        {"duration","Exit after this many seconds, useful for integration tests.","seconds"}
    });
    parser.process(app);
    auto fail=[](const char* message){std::cerr<<"qinda-patrol: "<<message<<'\n';return 2;};
    if(parser.isSet("demo")&&parser.isSet("no-metrics"))return fail("choose --demo or --no-metrics");
    if(parser.isSet("screensaver")&&parser.isSet("preview"))return fail("choose --screensaver or --preview");
    if(parser.isSet("all-screens")&&(!parser.isSet("screensaver")||parser.isSet("screen")))return fail("--all-screens requires --screensaver and cannot be combined with --screen");
    if(parser.isSet("list-screens")){describeScreens(QGuiApplication::screens());return 0;}
    patrol::WindowOptions options;options.screensaver=parser.isSet("screensaver");options.overlay=!parser.isSet("no-overlay");options.verbose=parser.isSet("verbose");
    bool ok=false;options.fps=parser.value("fps").toInt(&ok);if(!ok||options.fps<1||options.fps>60)return fail("--fps must be in 1..60");if(parser.isSet("eco"))options.fps=std::min(options.fps,20);
    options.seed=QRandomGenerator::system()->generate64();
    if(parser.isSet("seed")){QString seed=parser.value("seed");if(seed.startsWith('-'))return fail("invalid seed");options.seed=seed.toULongLong(&ok,0);if(!ok)return fail("invalid seed");}
    options.metricMode=parser.isSet("demo")?patrol::MetricMode::Demo:parser.isSet("no-metrics")?patrol::MetricMode::Hidden:patrol::MetricMode::Live;
    auto screens=QGuiApplication::screens();if(screens.isEmpty())return fail("no screens available");
    // On Wayland each screensaver window becomes a layer-shell surface bound to one output in the
    // overlay layer, so every monitor gets its own scene and docks stay beneath it. Compositors that
    // ignore the output hint of xdg-shell fullscreen (QindaQt does) still place these correctly.
    bool layerShell=false;
#ifdef PATROL_HAVE_LAYER_SHELL
    layerShell=options.screensaver&&!parser.isSet("no-layer-shell")&&QGuiApplication::platformName()==QLatin1String("wayland");
#endif
    QScreen* chosen=QGuiApplication::primaryScreen();
    if(parser.isSet("screen")){int index=parser.value("screen").toInt(&ok);if(!ok||index<0||index>=screens.size())return fail("invalid screen index");chosen=screens[index];}
    double duration=0;if(parser.isSet("duration")){duration=parser.value("duration").toDouble(&ok);if(!ok||!std::isfinite(duration)||duration<=0||duration>86400)return fail("duration must be in (0, 86400]");}
    std::unique_ptr<patrol::MetricSampler> sampler;
    if(options.metricMode==patrol::MetricMode::Live)sampler=std::make_unique<patrol::MetricSampler>(parser.value("interface").toStdString());
    struct OwnedWindow { QScreen* assigned; std::unique_ptr<patrol::PatrolWindow> window; };
    std::vector<OwnedWindow> windows;
    auto refreshActivity=[&](){if(sampler){bool exposed=std::any_of(windows.begin(),windows.end(),[](const auto& w){return w.window->isExposed();});sampler->setActive(exposed);}};
    std::uint64_t sequence=0;
    auto add=[&](QScreen* screen){auto config=options;config.index=int(sequence);config.seed+=sequence++*0x9e3779b97f4a7c15ULL;auto w=std::make_unique<patrol::PatrolWindow>(config,sampler.get(),screen);auto* ptr=w.get();ptr->exposureChanged=refreshActivity;windows.push_back({screen,std::move(w)});
        if(!options.screensaver){ptr->show();return;}
#ifdef PATROL_HAVE_LAYER_SHELL
        if(layerShell){auto* surface=LayerShellQt::Window::get(ptr);surface->setScope(QStringLiteral("screensaver"));surface->setLayer(LayerShellQt::Window::LayerOverlay);
            surface->setAnchors({LayerShellQt::Window::AnchorTop,LayerShellQt::Window::AnchorBottom,LayerShellQt::Window::AnchorLeft,LayerShellQt::Window::AnchorRight});
            surface->setExclusiveZone(-1);surface->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
            surface->setScreen(screen);surface->setWantsToBeOnActiveScreen(false);surface->setCloseOnDismissed(true);ptr->show();return;}
#endif
        ptr->showFullScreen();};
    if(options.verbose)std::cerr<<"qinda-patrol: "<<(layerShell?"layer-shell overlay surfaces":"xdg-shell fullscreen windows")<<'\n';
    // A screensaver covers every output unless one is chosen explicitly; each output gets its own world.
    bool everyScreen=options.screensaver&&!parser.isSet("screen");
    if(options.verbose)describeScreens(screens);
    if(everyScreen){for(auto* screen:screens)add(screen);
        QObject::connect(&app,&QGuiApplication::screenAdded,&app,[&](QScreen* screen){add(screen);});
        QObject::connect(&app,&QGuiApplication::screenRemoved,&app,[&](QScreen* screen){for(auto& w:windows)if(w.assigned==screen)w.window->exposureChanged={};std::erase_if(windows,[&](const auto& w){return w.assigned==screen;});refreshActivity();});
    }else add(chosen);
    if(duration>0)QTimer::singleShot(int(std::lround(duration*1000)),&app,&QGuiApplication::quit);
    int result=app.exec();for(auto& w:windows)w.window->exposureChanged={};windows.clear();return result;
}
