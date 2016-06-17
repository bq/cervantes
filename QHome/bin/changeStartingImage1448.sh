# !!!!! We expect bootlogo inlcuding the magic header for e60qh2 devices

echo "Executing script to change starting image"
if [ -f ../res/newImages/starting1448.raw4 ]; then
  echo "Changing starting image"
	dd if=../res/newImages/starting1448.raw4 of=/dev/mmcblk0 bs=512 seek=34815
  sync
	echo "Image changed"
fi

