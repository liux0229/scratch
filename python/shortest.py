import heapq

# edges: {from: [(to, weight)]}

def shortest(n, start, edges):
	dists = { i: float('inf') for i in range(n) }
	visited = set()
	heap = [(0, start)]
	dists[start] = 0
	while heap:
		dist, current = heapq.heappop(heap)
		if current in visited:
			continue
		visited.add(current)
		children = edges.get(current, [])
		for to, weight in children:
			nd = dist + weight
			if nd < dists[to]:
				heapq.heappush(heap, (nd, to))
				dists[to] = nd
	return dists

