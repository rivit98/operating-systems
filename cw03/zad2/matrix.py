import numpy as np

with open("./lista.txt", "rt") as f:
    listfile = f.readlines()

print(listfile)

for list_entry in listfile:
    e = list_entry.strip()
    parts = e.split()

    m1 = np.loadtxt(parts[0]).astype(int)
    m2 = np.loadtxt(parts[1]).astype(int)

    # print(m1)
    # print(m2)

    res_m = m1 @ m2
    # print(res_m.shape)

    with open('./out/{}_python.txt'.format(parts[2]), "wt") as f:
        for i in range(res_m.shape[0]):
            for j in range(res_m.shape[1]):
                f.write('{} '.format(res_m[i][j]))

            f.write('\n')