#!/bin/sh
# 
# This script installs WengoPhone 2.0

# Download WengoPhone
echo " "
echo "Password required to install in /opt directory... "
echo " "
cd /opt
sudo wget http://download.wengo.com/wengophone/rc/2006-10-03/WengoPhone-2.0-linux-bin-x86.tar.bz2

# Install WengoPhone 2.0
echo " "
echo "Installing WengoPhone 2.0"

# Extract the build
sudo tar -xjvf WengoPhone-2.0-linux-bin-x86.tar.bz2

# Renaming the folder
sudo mv wengophone-ng-binary-latest/ wengophone

# Make sure root owns the Swiftfox directory
sudo chown -hR root:root wengophone
#Applying permissions
sudo chmod 555 /opt/wengophone/*

# Add link to /usr/bin
sudo ln -s /opt/wengophone/wengophone.sh /usr/bin/qtwengophone

# Add WengoPhone menu Entry
cd /usr/share/applications
sudo wget http://download.wengo.com/wengophone/wengophone.desktop

# Copy wengophone icon
cp /opt/wengophone/pics/wengophone.png /usr/share/pixmaps/wengophone.png

# Download .protocol file
cd /usr/share/services
wget http://download.wengo.com/wengophone/wengophone.protocol

# Remove the downloaded tarball
sudo rm /opt/WengoPhone-2.0-linux-bin-x86.tar.bz2

echo " "
echo "WengoPhone 2.0 is now installed, enjoy your freedom to talk :) !"
echo " "
exit
