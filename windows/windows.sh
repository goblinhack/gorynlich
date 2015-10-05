"/c/Program Files (x86)/BitRock InstallBuilder Enterprise 9.5.3/bin/builder-cli.exe" build windows.xml 

cp /c/Users/nmcgill/Documents/InstallBuilder/output/gorynlich-0.01-windows-installer.exe ../gorynlich-installer.exe

if [ -d /c/Users/nmcgill/Google\ Drive/ ]; then
    cp ../gorynlich-installer.exe  /c/Users/nmcgill/Google\ Drive/
fi
