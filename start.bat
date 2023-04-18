START /B /WAIT cmd /c "install.bat"
START /B /WAIT cmd /c "build.bat"

START /D ballancer-server cmd /k "node ./ballancer-server-bundled.js"
START /D companion-server cmd /k "node ./companion-server-bundled.js"
START /D ui-typescript cmd /k "npm run start"

START "" http://localhost:9000/