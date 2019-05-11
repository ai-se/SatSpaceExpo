import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
from scipy.ndimage.filters import gaussian_filter1d
import bisect
import numpy as np
import pandas as pd
import pickle
import pdb

benchmark_models = ["Benchmarks/Blasted_Real/blasted_case47.cnf",
                    "Benchmarks/Blasted_Real/blasted_case110.cnf",
                    "Benchmarks/V7/s820a_7_4.cnf",
                    "Benchmarks/V15/s820a_15_7.cnf",
                    "Benchmarks/V3/s1238a_3_2.cnf",
                    "Benchmarks/V3/s1196a_3_2.cnf",
                    "Benchmarks/V15/s832a_15_7.cnf",
                    "Benchmarks/Blasted_Real/blasted_case_1_b12_2.cnf",
                    "Benchmarks/Blasted_Real/blasted_squaring16.cnf",
                    "Benchmarks/Blasted_Real/blasted_squaring7.cnf",
                    "Benchmarks/70.sk_3_40.cnf",
                    "Benchmarks/ProcessBean.sk_8_64.cnf",
                    "Benchmarks/56.sk_6_38.cnf",
                    "Benchmarks/35.sk_3_52.cnf",
                    "Benchmarks/80.sk_2_48.cnf",
                    "Benchmarks/7.sk_4_50.cnf",
                    "Benchmarks/doublyLinkedList.sk_8_37.cnf",
                    "Benchmarks/19.sk_3_48.cnf",
                    "Benchmarks/29.sk_3_45.cnf",
                    "Benchmarks/isolateRightmost.sk_7_481.cnf",
                    "Benchmarks/17.sk_3_45.cnf",
                    "Benchmarks/81.sk_5_51.cnf",
                    "Benchmarks/LoginService2.sk_23_36.cnf",
                    "Benchmarks/sort.sk_8_52.cnf",
                    "Benchmarks/parity.sk_11_11.cnf",
                    "Benchmarks/77.sk_3_44.cnf",
                    "Benchmarks/20.sk_1_51.cnf",
                    "Benchmarks/enqueueSeqSK.sk_10_42.cnf",
                    "Benchmarks/karatsuba.sk_7_41.cnf",
                    "Benchmarks/tutorial3.sk_4_31.cnf"]


infos = list()
names = list()
# for i in range(29):
#     with open(str(i) + '.INFO1', 'rb') as f:
#         info_raw = pickle.load(f)

#     info = pd.DataFrame(info_raw, columns=[
#                         'method', 'model', 'time', 'usols', 'ubits'])
#     names.append(info.iloc[0, 1])
#     infos.append(info)


# infos = pd.concat(infos)
# infos = infos.reset_index()
# infos['logtime'] = np.log10(infos.time)

# max_U = dict()
# for i, n in enumerate(names):
#     max_U[n] = max(
#         (infos.loc[(infos['model'] == n) & (infos['method'] == 'me')]).ubits)
# infos.maxU = infos.model.map(max_U)
# infos['u_ratio'] = infos.ubits/infos.maxU

# worthy = infos[infos['method'] == 'me']
# qs = infos[infos['method'] == 'qs']

# for name in names:
#     W = worthy[worthy.model == name]
#     Q = qs[qs.model == name]
#     xw = bisect.bisect_left(W.u_ratio.tolist(), 0.9)
#     xq = bisect.bisect_right(Q.u_ratio.tolist(), 0.9)
#     try:
#         print(Q.iloc[xq, 3] / W.iloc[xw, 3]-1, "\t", Q.iloc[xq, 3])
#     except:
#         print(Q.iloc[Q.shape[0]-1, 3] / W.iloc[W.shape[0]-1, 3] -
#               1, "\t", Q.iloc[Q.shape[0]-1, 3])


for benchmark_t in benchmark_models:
    benchmark = benchmark_t[benchmark_t.rfind('/')+1:]
    # me_info = pd.DataFrame(columns=['time', 'samples', 'ncd'])
    # qs_info = pd.DataFrame(columns=['time', 'samples', 'ncd'])
    me_info, qs_info = list(), list()
    with open(f'../memo/{benchmark}.me.report', 'r') as f:
        for line in f.readlines():
            if line[0] != '#':
                continue
            splits = line[:-1].split(' ')
            time, samples, ncd = float(splits[1]), int(
                splits[2]), float(splits[4])
            me_info.append([time, samples, ncd])

    with open(f'../memo/{benchmark}.qs.report', 'r') as f:
        for line in f.readlines():
            if line[0] != '#':
                continue
            splits = line[:-1].split(' ')
            time, samples, ncd = float(splits[1]), int(
                splits[2]), float(splits[4])
            qs_info.append([time, samples, ncd])

    ME = pd.DataFrame(me_info, columns=['time', 'samples', 'ncd'])
    QS = pd.DataFrame(qs_info, columns=['time', 'samples', 'ncd'])

    # ME.plot(x='time', y='ncd')
    # QS.plot(x='time', y='ncd')
    # plt.plot(ME.time, ME.ncd, label='WORTHY')
    # plt.plot(QS.time, QS.ncd, label='QS')
    # plt.semilogx()
    # plt.legend()
    # plt.title(benchmark)
    # plt.show()
    # # break
    if not ME.shape[0] or not QS.shape[0]:
        continue
    startat = min(min(ME.ncd), min(QS.ncd))
    endat = max(max(ME.ncd), max(QS.ncd))
    print(benchmark, end=' ', flush=True)
    for r in [0.95]:
        splitting = (endat - startat) * r + startat
        # id1 = bisect.bisect_left(ME.ncd, splitting)
        id2 = bisect.bisect_left(QS.ncd, splitting)
        # if id1 >= ME.shape[0]:
        id1 = ME.shape[0] - 1
        if id2 >= QS.shape[0]:
            id2 = QS.shape[0] - 1
        print(ME.iloc[id1, 1], QS.iloc[id2, 1], '?', end=' ', flush=True)
    print()
