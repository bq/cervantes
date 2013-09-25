echo "Executing script to change starting image"
if [ -f ../res/newImages/starting800.raw4 ]; then
  echo "Changing starting image"
  dd if=../res/newImages/starting800.raw4 of=/dev/mmcblk0 bs=512 seek=18432
  sync
  echo "Image changed"
fi

