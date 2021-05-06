#!/bin/bash
rive_build_osx()
{
	java -jar /Users/jhonny/Downloads/bob.jar build  --build-server http://localhost:9000 --platform x86_64-darwin --variant debug --with-symbols debug
	chmod +x /Users/jhonny/dev/defold-rive-test/build/x86_64-osx/dmengine
}

rive_build_windows()
{
	java -jar /Users/jhonny/Downloads/bob.jar build  --build-server http://localhost:9000 --platform x86_64-win32 --variant debug --with-symbols debug
}

rive_build_android_64()
{
	java -jar /Users/jhonny/Downloads/bob.jar build  --build-server http://localhost:9000 --platform arm64-android --variant debug --with-symbols debug
}

rive_build_android_32()
{
	java -jar /Users/jhonny/Downloads/bob.jar build  --build-server http://localhost:9000 --platform armv7-android --variant debug --with-symbols debug
}

