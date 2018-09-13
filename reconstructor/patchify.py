import os
import subprocess

exe_dir = r"C:\phd\ControlledConvolution\ControlledConvolution\x64\Release"
exe = "ControlledConvolution.exe"
dataset= r"E:\DATA\cifar\cifar100\cifar100\train"
oDir = r"E:\DATA\cifar\cifar100\preprocessed"
patch_height = ['16','4','32']
resize = "32"
order = '0'


#ControlledConvolution.exe --iDir="E:\DATA\cifar\cifar100\cifar100\train" --oDir="E:\DATA\cifar\cifar100\preprocessed\train\airplane" 
# --measure=ae --patch_height=16 --patch_width=16 --resize=32 --order=0

categories = os.listdir(dataset)

for cat in categories:
    iDir = os.path.join(dataset,cat)
    args = "--iDir= \"{}\" --oDir=\"{}\" --measure=ae --patch_height=16 --patch_width=16 --resize={} --order={}".format(iDir,oDir,resize,order)
    print (args)
    sdfsdfclear
    subprocess.Popen(args,executable=os.path.join(exe_dir,exe))
    # args = "--iDir= \"{}\" --oDir=\"{}\" --measure=ae --patch_height={} --patch_width={} --resize={} --order={}".format(iDir,oDir,patch_height[0],patch_height[0],resize,order)
    # subprocess.Popen(args,executable=os.path.join(exe_dir,exe))
    # args = "--iDir= \"{}\" --oDir=\"{}\" --measure=ae --patch_height={} --patch_width={} --resize={} --order={}".format(iDir,oDir,patch_height[2],patch_height[2],resize,order)
    # subprocess.Popen(args,executable=os.path.join(exe_dir,exe))