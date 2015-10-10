
"/c/Program Files (x86)/BitRock InstallBuilder Enterprise 15.9.0/bin/builder-cli.exe" build windows.xml 

cp /c/Users/benten/Documents/InstallBuilder/output/gorynlich-0.01-windows-installer.exe ../gorynlich-installer.exe

if [ -d /c/Users/benten/Google\ Drive/ ]; then
    cp ../gorynlich-installer.exe  /c/Users/benten/Google\ Drive/
fi
