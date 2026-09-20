#!/usr/bin/env python3
"""Create immutable local Git sources and versioned Gentoo ebuilds.

Does not emerge packages, edit /etc/portage, or change MAKEOPTS.
"""
from pathlib import Path
import json,shutil,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
versions={'qinda-patrol':'0.2.0','circuit-reef':'1.1.0','prism-circuit':'1.1.0','prism-brawl':'1.1.0','starward-reimagined':'2.1.0'}
repo=Path('/home/cabewse/git/screensaver-suite.git')
def git(*args,cwd=None):return subprocess.check_output(['git',*args],cwd=cwd,text=True).strip()
with tempfile.TemporaryDirectory(prefix='screensaver-source-') as temp:
 stage=Path(temp)
 for project in versions:
  target=stage/project;target.mkdir()
  for sub in ['src','include','compat','shaders','qt','integration','packaging','tests','tools']:
   src=root/project/sub
   if src.exists():shutil.copytree(src,target/sub)
  for name in ['CMakeLists.txt','CMakePresets.json','LICENSE','README.md']:
   if (root/project/name).exists():shutil.copy2(root/project/name,target/name)
  for sub in ['assets','docs']:
   for source in (root/project/sub).rglob('*'):
    if source.is_file() and source.suffix in ['.md','.svg','.json']:
     dest=target/source.relative_to(root/project);dest.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(source,dest)
 shutil.copytree(root/'common',stage/'common');shutil.copy2(root/'README.md',stage/'README.md')
 (stage/'tools').mkdir();shutil.copy2(root/'tools/build-suite.py',stage/'tools/build-suite.py')
 git('init','-q',cwd=stage);git('add','.',cwd=stage)
 git('-c','user.name=Codex','-c','user.email=codex@localhost','commit','-q','-m','Refine five screensavers, native monitor lifecycle and synthesized SFX',cwd=stage)
 commit=git('rev-parse','HEAD',cwd=stage)
 if not repo.exists():git('init','--bare','-q',str(repo))
 git('push','-q',str(repo),f'HEAD:refs/heads/refinement-{commit[:12]}',cwd=stage)
 for project,version in versions.items():
  depends='\n\t>=dev-qt/qtbase-6.4:6=[gui,wayland]\n\tkde-plasma/layer-shell-qt:6='
  if project in ['qinda-patrol','circuit-reef']:depends+='\n\t>=dev-qt/qtdeclarative-6.4:6='
  if project!='qinda-patrol':depends+='\n\t>=media-libs/libsdl2-2.0.18[wayland]\n\tx11-libs/cairo\n\tvirtual/opengl'
  options=['-DBUILD_SHARED_LIBS=OFF','-DBUILD_TESTING=OFF']
  if project=='qinda-patrol':options+=['-DPATROL_BUILD_QML_MODULE=ON','-DPATROL_BUILD_DESKTOP=ON','-DPATROL_BUILD_QUICK=OFF','-DPATROL_LAYER_SHELL=ON']
  if project=='circuit-reef':options+=['-DREEF_BUILD_QT=ON','-DREEF_BUILD_QML_MODULE=ON']
  ebuild='''# Copyright 2026 QindaQt contributors
# Distributed under the terms of the GNU General Public License v2
EAPI=8
inherit cmake xdg git-r3
DESCRIPTION="Qinda '''+project+''' refined native screensaver"
HOMEPAGE="https://github.com/Es00bac/QindaQt"
EGIT_REPO_URI="file://'''+str(repo)+'''"
EGIT_COMMIT="'''+commit+'''"
EGIT_CHECKOUT_DIR="${WORKDIR}/${P}"
S="${EGIT_CHECKOUT_DIR}/'''+project+'''"
LICENSE="GPL-3+"
SLOT="0"
KEYWORDS="~amd64"
RDEPEND="'''+depends+'''\n"
DEPEND="${RDEPEND}"
BDEPEND="virtual/pkgconfig"
# Portage's configured MAKEOPTS is inherited unchanged.
src_configure() {
 local mycmakeargs=(
'''+''.join('  '+opt+'\n' for opt in options)+''' )
 cmake_src_configure
}
'''
  dest=root/'packaging/ebuilds/x11-misc'/project;dest.mkdir(parents=True,exist_ok=True);(dest/f'{project}-{version}.ebuild').write_text(ebuild)
  (dest/'metadata.xml').write_text('<?xml version="1.0" encoding="UTF-8"?>\n<!DOCTYPE pkgmetadata SYSTEM "https://www.gentoo.org/dtd/metadata.dtd">\n<pkgmetadata><longdescription>Locally maintained native Qinda screensaver from an immutable source snapshot.</longdescription></pkgmetadata>\n')
 (root/'packaging/source-lock.json').write_text(json.dumps({'repository':str(repo),'commit':commit,'versions':versions},indent=2)+'\n')
 print(commit)
