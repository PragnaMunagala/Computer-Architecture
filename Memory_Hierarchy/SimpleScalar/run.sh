mkdir ~/cse420/seqread
mkdir ~/cse420/seqwrite
mkdir ~/cse420/ranread
mkdir ~/cse420/ranwrite


for i in 1 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 16384 32768 65536
do
simplesim-3.0/sim-outorder -cache:il1 il1:1024:16:2:l -cache:dl1 dl1:512:32:4:l -cache:dl2 ul2:256:64:16:l ~/cse420/seqread1 $i &> ~/cse420/seqread/$i.txt

simplesim-3.0/sim-outorder -cache:il1 il1:1024:16:2:l -cache:dl1 dl1:512:32:4:l -cache:dl2 ul2:256:64:16:l ~/cse420/seqwrite1 $i &> ~/cse420/seqwrite/$i.txt

simplesim-3.0/sim-outorder -cache:il1 il1:1024:16:2:l -cache:dl1 dl1:512:32:4:l -cache:dl2 ul2:256:64:16:l ~/cse420/ranread1 $i &> ~/cse420/ranread/$i.txt

simplesim-3.0/sim-outorder -cache:il1 il1:1024:16:2:l -cache:dl1 dl1:512:32:4:l -cache:dl2 ul2:256:64:16:l ~/cse420/ranwrite1 $i &> ~/cse420/ranwrite/$i.txt
done







