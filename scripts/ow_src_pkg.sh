#!/bin/sh
#
# auto_src_pkg: checkout sources at a date & create an archive
#   It Can be used to create nightly svn snapshots.
#

output_dir="wengophone-2.1"

#remove the last archive
rm -f $output_dir.tar.bz2
rm -f $output_dir.md5

# Download sources from svn
echo -n "Downloading sources... "
svn export -q --password guest --username guest http://dev.openwengo.org/svn/openwengo/wengophone-ng/branches/wengophone-2.1/  $output_dir
echo "done"

#TODO remove more useless stuff
rm -rf $output_dir/libs/3rdparty/ffmpeg/binary-lib/
rm -rf $output_dir/wengophone/nsis/dll/
rm -rf $output_dir/libs/3rdparty/openssl/binary-lib
rm -rf $output_dir/wengophone/together/

# Make the sources archive
echo -n "Writing archive... "
tar cjf $output_dir.tar.bz2 $output_dir
echo "done"

#generate a MD5
md5=`md5sum $output_dir.tar.bz2`
echo "MD5 checksum: $md5" | tee $output_dir.md5

#remove the folder
rm -rf $output_dir
