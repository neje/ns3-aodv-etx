# ns3-aodv-etx
This project implements ETX metrix in AODV routing protocol in NS3 simulator.
Project is work in progress. Updated to work with 3.28 release.

To try how it works, you should do the following:
1) Delete all files from folder /ns-3.28/src/aodv/model in your ns-3 instalation (save these files if you want to restore them later);
2) Then copy all files from folder aodv/model from this repository to folder /ns-3.28/src/aodv/model in your ns-3 instalaltion;
3) Do the same with file wscript from folder aodv from this repository to folder /ns-3.28/src/aodv in your ns-3 instalaltion;
4) Write a script that uses aodv routing protocol and you are ready to go. You can use script file provided for you in folder scratch. Put it in your scratch folder and run.

