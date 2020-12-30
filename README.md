# FactoryMicTest
唤醒，一致性产测工具
该应用使用c++对接百度so
该应用用于测试C12 百度DSP唤醒和一致性
由于没有root权限，需手动修改权限

1. kill tcl_voice 两个进程
2. chown system.system /dev/spidev32766.0   chomd 777 /dev/spidev32766.0
3. chown system.system /dev/FFwakeup   chomd 777 /dev/FFwakeup
4. setenforce 0 关闭selinux权限
