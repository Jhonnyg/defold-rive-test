#!/bin/bash
build_osx()
{
	java -jar /Users/jhonny/Downloads/bob.jar build  --build-server http://localhost:9000 --platform x86_64-darwin --variant debug --with-symbols debug
	chmod +x /Users/jhonny/dev/defold-rive-test/build/x86_64-osx/dmengine
}

build_windows()
{
	java -jar /Users/jhonny/Downloads/bob.jar build  --build-server http://localhost:9000 --platform x86_64-win32 --variant debug --with-symbols debug
}
