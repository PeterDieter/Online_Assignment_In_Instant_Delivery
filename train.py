import subprocess


lambdas = [0, 0.9, 0.99, 0.999, 1]
# for complex commands, with many args, use string + `shell=True`:
for lam in lambdas:
    subprocess.run(["./onlineAssignment", "instances/instance_900_8_3_30_120_60_train.txt", "trainREINFORCE", str(lam)])