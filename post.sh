cp kernel img/boot/exe
echo "Performing post-build actions:"
echo "Creating raw img file for writing to a drive:"
./mkbootimg.exe chroma.json chroma.img # for linux remove the .exe

echo "Checking for VirtualBox management tools"
if [ -x "$(command -v VBoxManage)" ]; then
echo "  VBoxManage found.. integrating."
echo "   Clearing VirtualBox cache:"
VBoxManage storageattach Chroma --port 0 --storagectl AHCI --medium none # removing a drive in virtualbox = attaching nothing
VBoxManage closemedium disk --delete "`pwd`/chroma.vdi" # remove the existing .vdi image from the registry so we can overwrite it
echo "   Creating new VirtualBox VDI image:"
VBoxManage convertfromraw chroma.img --format VDI chroma.vdi # generate the new vdi from the img
echo "   Attaching image to the VM."
VBoxManage storageattach Chroma --port 0 --storagectl AHCI --type hdd --medium "`pwd`/chroma.vdi" # attach the new vdi to the vm so we can run it
echo "   VirtualBox integrations complete. You may execute run.sh to run your VirtualBox VM."
fi

