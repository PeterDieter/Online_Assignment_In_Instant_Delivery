import subprocess


lambdasTemporal = [0.99]
lambdasSpatial = [0.85]
# for complex commands, with many args, use string + `shell=True`:
for lamT in lambdasTemporal:
    for lamS in lambdasSpatial:
        subprocess.run(["./onlineAssignment", "instances/instance_900_5_3_25_180_60_train.txt", "trainREINFORCE", str(lamT), str(lamS)])