V8 JavaScript Engine
=============

V8 is Google's open source JavaScript engine.

V8 implements ECMAScript as specified in ECMA-262.

V8 is written in C++ and is used in Google Chrome, the open source
browser from Google.

V8 can run standalone, or can be embedded into any C++ application.

V8 Project page: https://v8.dev/docs


Getting the Code
=============

Checkout [depot tools](http://www.chromium.org/developers/how-tos/install-depot-tools), and run

        fetch v8

This will checkout V8 into the directory `v8` and fetch all of its dependencies.
To stay up to date, run

        git pull origin
        gclient sync

For fetching all branches, add the following into your remote
configuration in `.git/config`:

        fetch = +refs/branch-heads/*:refs/remotes/branch-heads/*
        fetch = +refs/tags/*:refs/tags/*


Contributing
=============

Please follow the instructions mentioned at
[v8.dev/docs/contribute](https://v8.dev/docs/contribute).

windows下如何只下载和build指定的分支

Windows 设置环境变量：
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set vs2022_install=E:\Program Files\Microsoft Visual Studio\2022\Community

设置环境变量：
set WINDOWSSDKDIR=E:\Windows Kits\10

mkdir chromium && cd chromium
mkdir src && cd src
git init
git fetch https://github.com/heartup/chromium.git +refs/tags/141.0.7357.0:chromium_141.0.7357.0 --depth 1
git checkout tags/141.0.7357.0

cd ..
fetch chromium  (中断并修改 .gclient 配置文件中的git地址为自己的fork)  // 或者 gclient config --unmanaged  https://github.com/heartup/chromium.git
gclient sync  // 同步依赖

cd src
gn gen out\Default
autoninja -C out\Default chrome
