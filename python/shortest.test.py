import shortest

n = 8
start = 0
edges = {0: [(1, 1), (2, 3)], 1: [(2, 1)], 3: [(0, 1)], 2: [(4, 2), (5, 10)], 4: [(6, 2)], 5: [(7, 4)], 6: [(7, 2), (5, 0) ]}

dists = shortest.shortest(n, start, edges)
print(dists)
