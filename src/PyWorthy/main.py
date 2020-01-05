#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2019, Jianfeng Chen <jchen37@ncsu.edu>
# vim: set ts=4 sts=4 sw=4 expandtab smartindent:
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights
#  to use, copy, modify, merge, publish, _distribute, sublicense, and/or sell
#  copies of the Software, and to permit persons to whom the Software is
#  furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
#  THE SOFTWARE.

from __future__ import division

import sys
import os
from z3 import *
from BitVector import *
from sklearn.cluster import KMeans
import time
import numpy as np
import pdb

path = os.getcwd()
rootpath = path[:path.rfind('src') + 4]
sys.path.append(f'{rootpath}/PyWorthy')

benchmark_models = [
    "Benchmarks/Blasted_Real/blasted_case47.cnf",  # 0
    "Benchmarks/Blasted_Real/blasted_case110.cnf",
    "Benchmarks/V7/s820a_7_4.cnf",
    "Benchmarks/V15/s820a_15_7.cnf",
    "Benchmarks/V3/s1238a_3_2.cnf",
    "Benchmarks/V3/s1196a_3_2.cnf",  # 5
    "Benchmarks/V15/s832a_15_7.cnf",
    "Benchmarks/Blasted_Real/blasted_case_1_b12_2.cnf",
    "Benchmarks/Blasted_Real/blasted_squaring16.cnf",
    "Benchmarks/Blasted_Real/blasted_squaring7.cnf",
    "Benchmarks/70.sk_3_40.cnf",  # 10
    "Benchmarks/ProcessBean.sk_8_64.cnf",
    "Benchmarks/56.sk_6_38.cnf",
    "Benchmarks/35.sk_3_52.cnf",
    "Benchmarks/80.sk_2_48.cnf",
    "Benchmarks/7.sk_4_50.cnf",  # 15
    "Benchmarks/doublyLinkedList.sk_8_37.cnf",
    "Benchmarks/19.sk_3_48.cnf",
    "Benchmarks/29.sk_3_45.cnf",
    "Benchmarks/isolateRightmost.sk_7_481.cnf",
    "Benchmarks/17.sk_3_45.cnf",  # 20
    "Benchmarks/81.sk_5_51.cnf",
    "Benchmarks/LoginService2.sk_23_36.cnf",
    "Benchmarks/sort.sk_8_52.cnf",
    "Benchmarks/parity.sk_11_11.cnf",
    "Benchmarks/77.sk_3_44.cnf",  # 25
    "Benchmarks/20.sk_1_51.cnf",
    "Benchmarks/enqueueSeqSK.sk_10_42.cnf",
    "Benchmarks/karatsuba.sk_7_41.cnf",
    # "Benchmarks/diagStencilClean.sk_41_36.cnf",
    "Benchmarks/tutorial3.sk_4_31.cnf"
]


class program_s:
    def __init__(self, name):
        self.name = name
        self.var_num = 0
        self.clauses_num = 0
        self.vars = list()
        self.clauses = list()
        self.cnfs = list()
        self.var_id = dict()
        self.var_to_clause_map = dict()

    def verify(self, sol, diff):
        """
        sol and diff are BitVector
        """
        to_check_clause_id = []
        for i, v in enumerate(diff):
            if v:
                to_check_clause_id.extend(self.var_to_clause_map[i + 1])
        to_check_clause_id = set(to_check_clause_id)
        for cid in to_check_clause_id:
            for v in self.cnfs[cid]:
                if v == 0:
                    return False
                if v > 0 and sol[self.var_id[v]]:
                    break
                if v < 0 and not sol[self.var_id[-v]]:
                    break

        return True


def load_cnf(model_file, model_name):
    program = program_s(model_name)
    clause_adding_id = 0

    with open(model_file, 'r') as f:
        for line in f.readlines():
            line = line.strip()
            if line.startswith('c'):
                continue
            if line.startswith('p'):
                _1, _2, var_num, clause_num = line.split(' ')
                program.var_num, program.clauses_num = int(var_num), int(
                    clause_num)
                continue
            vs = list(map(lambda i: int(i), line.split(' ')))

            for v in vs[:-1]:
                if abs(v) not in program.var_to_clause_map:
                    program.var_to_clause_map[abs(v)] = [clause_adding_id]
                else:
                    program.var_to_clause_map[abs(v)].append(clause_adding_id)
            program.cnfs.append(vs)
            clause_adding_id += 1

        used_vars = sorted(program.var_to_clause_map.keys())
        for i in used_vars:
            program.vars.append(Bool(f'v{i}'))
        for i, v in enumerate(used_vars):
            program.var_id[v] = i

        for cnf in program.cnfs:
            vs_ref = [
                program.vars[program.var_id[v]] if v > 0 else
                (Not(program.vars[program.var_id[-v]])) for v in cnf[:-1]
            ]
            program.clauses.append(Or(*vs_ref))

    return program


def load_over_sampling(model_name):
    f_name = f'{rootpath}../memo/{model_name}.cnf.memo'
    inits = list()
    with open(f_name, 'r') as f:
        for line in f.readlines():
            line = line.strip()
            bv = BitVector(bitstring=line)
            inits.append(bv)
    return inits


def worthy(program, inits, out, RANDOM_SEED=0):
    N_CLUSTERS = 10

    # get N_CLUSTERS (10) cluster, randomly select one as center
    kmeans = KMeans(n_clusters=N_CLUSTERS, random_state=RANDOM_SEED).fit(inits)
    centers = [
        np.random.choice(np.where(kmeans.labels_ == ci)[0])
        for ci in range(N_CLUSTERS)
    ]

    # get the deltas of all pairs
    deltas_strs = dict()
    for i in range(len(inits)):
        out.write(str(inits[i]) + '\n')
        for j in range(i + 1, len(inits)):
            a, b = inits[i], inits[j]
            ds = str(a ^ b)
            if ds not in deltas_strs:
                deltas_strs[ds] = 1
            else:
                deltas_strs[ds] += 1
    # print("INIT DELTA DONE")

    deltas = list()
    for k, v in deltas_strs.items():
        deltas.append([BitVector(bitstring=k), v])
    deltas = sorted(deltas, key=lambda i: i[1], reverse=True)
    freqs = np.array([i[1] for i in deltas])
    freqs = freqs / freqs.sum()
    # setting up near the deltas
    totalSamples, validSamples = 0, 0
    for pfi in centers:
        PFx = inits[pfi]
        life = 100
        max_try = 100
        while life and max_try:
            # print(f'{life} ', end='')
            max_try -= 1
            ai = np.random.choice(
                len(freqs), np.random.choice([1, 2]), p=freqs)
            delta = deltas[ai[0]][0]
            if len(ai) > 1:
                delta = delta | deltas[ai[1]][0]
            new_sample = PFx ^ delta
            # pdb.set_trace()
            ispass = program.verify(new_sample, delta)
            totalSamples += 1
            life -= 0 if ispass else 1
            if ispass:
                validSamples += 1
                out.write(str(new_sample) + '\n')
    return validSamples, totalSamples


if __name__ == '__main__':
    # print("this is the main entrance of worthy")
    startAt = time.time()
    # parse CNF
    model_id = 3
    for argi, arg in enumerate(sys.argv):
        if arg == '-i':
            model_id = int(sys.argv[argi + 1])
    model_file = f'{rootpath}../{benchmark_models[model_id]}'
    model_name = model_file[model_file.rfind('/') + 1:model_file.rfind('.cnf')]
    program = load_cnf(model_file, model_name)
    # print(f"LOADING {model_name} DONE.")
    # get init sampling
    inits = load_over_sampling(model_name)

    # starting worthy
    outputfile = open(f'{rootpath}../memo/{model_name}.cnf.me.valid2', 'a+')
    validSamples, totalSamples = worthy(program, inits, outputfile)
    print(f"{model_name} {validSamples} {totalSamples}")
    execTime = round(time.time() - startAt, 3)
    outputfile.write(f'# {execTime} {validSamples}/{totalSamples}')

    outputfile.close()

    # s = Solver()
    # for clause in program.clauses:
    #     s.add(clause)
    # s.check()
    # mod = s.model()
    # pdb.set_trace()
    # print(s.model())