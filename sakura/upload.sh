#!/bin/sh
mount /mnt/sde1 || exit 1
cp uip.bin /mnt/sde1 || exit 2
sync
umount /mnt/sde1
