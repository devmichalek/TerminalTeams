
# VirtualBox Setup
By using VirtualBox, you can connect ISO files in at least two ways through the VirtualBox Manager or from the virtual machine interface when the guest operating system is running. Install VirtualBox:
```
sudo apt-get install virtualbox
```

## VM Basic Setup
1. Download Ubuntu 22.04 LTS Desktop from this [site](https://ubuntu.com/download/desktop). Always use original ISO file, like from the original Red Hat [site](https://developers.redhat.com/products/rhel/download).
2. Open VirtualBox Manager
   ```
   virtualbox &> /dev/null &
   ```
3. Click on "New" and start creating new virtual machine (<a href="./doc/4.19.1.png">screenshoot</a>).
4. Enter virtual machine name, directory. Select "Linux" type and version "Red Hat (64-bit)" (<a href="./doc/4.19.2.png">screenshoot</a>).
5. Choose how much RAM will be dedicated to the virtual machine (<a href="./doc/4.19.3.png">screenshoot</a>).
6. Create virtual hard drive (dynamic VDI) and select an already existing one (<a href="./doc/4.19.4.png">screenshoot</a>).
   In case of creating new one it is better to choose dynamic VDI type with at least 20GB of memory.
7. At this point virtual machine is created. Go to the "Settings" of newly created VM (<a href="./doc/4.19.5.png">screenshoot</a>).
8. Go to "Storage". Under "Storage Drives" section, select the disc "Empty" item and choose downloaded ISO file to be opened (<a href="./doc/4.19.6.png">screenshoot</a>).
   From this point forward ISO is now mounted.
9. Start newly created VM and complete all the steps during system installation (it differs among systems). Once you've completed all the steps, the installer will prompt you to reboot the machine. Decline and stop the VM by yourself. Now go to the "Storage" -> "Storage Devices" and remove previously added ISO file. Save the settings and enjoy the installed system. ISO file is needed only at the beginning and that`s the reason why we should remove it after system installation.

## VM's Network Setup
In VirtualBox Manager go to VM's "Settings"->"Network"->"Adapter 1" tab. Click on "Enable Network Adapter", change "Attached to" to "Bridged Adapter" and choose NIC with access to the Internet.

## VM's OS Setup
In VirtualBox Manager:
1. Start Virtual Machine (optionally clone it before start).
2. Set the password for root user.
    ```
    sudo passwd root
    ```
3. Unlock the root account.
    ```
    sudo passwd -u root
    ```
4. Edit `sudo nano /etc/gdm3/custom.conf`, and add the following line under `[security]`.
    ```
    AllowRoot=true
    ```
5. Edit `sudo nano /etc/pam.d/gdm-password`, and comment out the following line by adding a `#` in front of it.
    ```
    #auth   required    pam_succeed_if.so user != root quiet_success
    ```
6. Enable short passwords. Edit `sudo nano /etc/security/pwquality.conf`.
7. Test root account by rebooting VM. Select "Not Listed" at the login screen, then type "root" in the username field, and your root password in the password field.
8. Optionally (if VM was cloned) change username, home directory, hostname using root account.
    ```
    sudo usermod -l newUsername oldUsername
    sudo groupmod -n newUsername oldUsername
    sudo usermod -d /home/newHomeDir -m newUsername
    ```
    To change hostname go to "Settings"->"About".
9. Setup tmux on the guest.
    ```
    sudo apt update
    sudo apt install -y build-essential linux-headers-$(uname -r)
    sudo apt install tmux
    ```

## VM's Clipboard Setup
On the host type:
```
sudo apt-get install virtualbox-guest-additions-iso
```
Under guest (VM) "Settings"->"Storage"->"Controller: IDE" insert installed CD image then run the VM.
Go the directory with mounted image and type:
```
./autorun.sh
```
Restart the VM. In VirtualBox Manager click on VM's "Settings"->"General"->"Advanced" then set "Shared Clipboard" to "Host to Guest", same for "Drag'n'Drop". Lastly under guest (VM) "Settings"->"Storage"->"Controller: IDE" remove previously attached image.

## VM's Shared Folder Setup
On the host type:
```
VBoxManage sharedfolder add "VM name" --name "sharename" --hostpath "/home/$USER/VM/shared" --automount
```
Start VM and then on the guest type:
```
sudo usermod -aG vboxsf $USER
reboot
cd /media
ls -la
```

## VM's Private Network Setup
You can now create a virtual network in VirtualBox between two virtual machines.

1. In VirtualBox Manager go to "File"->"Host Network Manager" and click on "Create".
2. Click on "Properties" of newly created private network.
3. In "DHCP Server" click on "Enable Server" and set below properties:
    ```
    Server Address: 192.168.56.2
    Server Mask: 255.255.255.0
    Lower Address Bound: 192.168.56.3
    Upper Address Bound: 192.168.56.254
    ```
4. In "Adapter" click on "Configure Adapter Manually" with the below settings:
    ```
    IPv4 Address: 192.168.56.3
    IPv4 Network Mask: 255.255.255.0
    ```
    Click on "Apply".
5. Go to "Settings" of VM. Then on "Network" open "Adapter 2" tab and click on "Enable Network Adapter". Change "Attached to" to "Host-only Adapter" and choose previously created adapter.
6. Do the same for other virtual machines - set "Adapter 2".
7. Check VM IP addresses using `ip addr show` then check if VMs see each other on the network using `ping` command.

# Running Tests
ToDo