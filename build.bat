cd companion-server
START /B /WAIT cmd /c "npm run pack-server"
cd ..

cd ballancer-server
START /B /WAIT cmd /c "npm run pack-server"
cd ..