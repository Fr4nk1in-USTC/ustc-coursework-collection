#!/usr/bin/bash
gcc -O2 -o bit_flip bit_flip.c

echo ""
echo "Initializing backup file..."
cp fat16.img fat16.img.bak
cp fat16.img fat16.img.bak2
echo ""

echo "Origin MD5 sum:"
md5sum fat16.img
md5sum fat16.img.bak
md5sum fat16.img.bak2
echo ""

echo "Calling ./bit_flip"
echo ""
if [[ $# -eq 0 ]]; then
    ./bit_flip
elif [[ $# -eq 1 ]]; then
    ./bit_flip $1
    exit 0
elif [[ $# -eq 4 ]]; then
    ./bit_flip $1 $2 $3 $4
else
    echo "Invalid arguments"
    exit 1
fi
echo ""

echo ""
echo "New MD5 sum:"
md5sum fat16.img
md5sum fat16.img.bak
md5sum fat16.img.bak2
echo ""

echo ""
while true; do
    read -p "Continue Mounting Image WITHOUT Backup Check? (Y/n)" yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y(es) or n(o).";;
    esac
done

make default;
./simple_fat16 -s -d ./fat_dir
echo ""

while true; do
    read -p "Continue Mounting Image WITH Backup Check? (Y/n)" yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y(es) or n(o).";;
    esac
done

make backup;
./simple_fat16 -s -d ./fat_dir

echo ""

echo "Recheck MD5 sum:"
md5sum fat16.img
md5sum fat16.img.bak
md5sum fat16.img.bak2