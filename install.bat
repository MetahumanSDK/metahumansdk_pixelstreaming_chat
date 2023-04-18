cd ballancer-server
START /B /WAIT cmd /c "npm install --verbose"
cd ..

cd companion-server
START /B /WAIT cmd /c "npm install --verbose"
cd ..

cd ui-typescript
START /B /WAIT cmd /c "npm install --verbose"
cd ..