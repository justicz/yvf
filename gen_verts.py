from collections import defaultdict, deque
from PIL import Image
import os, sys, struct, math, random, json
from multiprocessing import Pool


class ObjectFile:
	def __init__(self, obj_file):
		self.obj_file = obj_file
		self.parse()

	def parse(self):
		vertices = []
		faces = []
		bunny = False
		self.all_vertices = set()
		with open(self.obj_file) as fin:
			for line in fin.readlines():
				l = line.split(" ")
				#if l[0] == "o" and l[1][0] == "b":
				#	bunny = True
				if l[0] == "v":
					x, y, z = l[1:]
					x, y, z = float(x), float(y), float(z)
					vertices.append((x, y, z))
				#if not bunny:
				#	continue
				if l[0] == "f":
					indices = [a.split("//")[0] for a in l]
					i0, i1, i2 = indices[1:]
					i0, i1, i2 = int(i0), int(i1), int(i2)
					self.all_vertices.add(i0 - 1)
					self.all_vertices.add(i1 - 1)
					self.all_vertices.add(i2 - 1)
					faces.append((i0 - 1, i1 - 1, i2 - 1))

		self.vertices = vertices
		self.face_indices = faces

	def walk(self):
		graph = defaultdict(set)
		queue = deque()
		visited = set()

		# Generate the graph
		for f0, f1, f2 in self.face_indices:
			graph[f0] |= set([f1, f2])
			graph[f1] |= set([f0, f2])
			graph[f2] |= set([f0, f1])

		# Walk the graph
		key = self.all_vertices.pop()
		queue.append(key)
		visited.add(key)
		self.inverse_walk = {}
		out = []
		i = 0
		num_comp = 0
		while True:
			# check if we're done
			if len(queue) == 0:
				num_comp += 1
				self.all_vertices -= visited
				visited = set()
				if len(self.all_vertices) == 0:
					print num_comp, "connected components"
					return out
				queue.append(self.all_vertices.pop())
			key = queue.popleft()
			out.append(key)
			self.inverse_walk[key] = i
			children = graph[key]
			for child in children:
				if child not in visited:
					queue.append(child)
					visited.add(child)
			i += 1

def job(start_frame):
	end_frame = min(start_frame + BATCH_SIZE + 1, NUM_FRAMES)
	for fn in range(start_frame + 1, end_frame):
		#o = ObjectFile(indirectory+"/bunbun_multiple_buns_100_{}.obj".format(str(fn).zfill(6)))
		o = ObjectFile(indirectory+"/bunbun_{}.obj".format(str(fn).zfill(6)))
		filename = "{}/out0_{}.txt".format(outdirectory, str(fn).zfill(6))
		with open(filename, "w") as fout:
			for n, verti in enumerate(WALK):
				row = n // WIDTH
				col = n % WIDTH
				writex, writey = col, row
				x, y, z = o.vertices[verti]
				fout.write("{}\n{}\n{}\n".format(repr(x), repr(y), repr(z)))
		print "wrote {}".format(filename)

if __name__ == "__main__":
	if len(sys.argv) != 3:
		print("Need indir, outdir")
		sys.exit(-1)

	indirectory = sys.argv[1]
	outdirectory = sys.argv[2]
	#temp_o = ObjectFile(indirectory+"/bunbun_multiple_buns_100_000001.obj")
	temp_o = ObjectFile(indirectory+"/bunbun_000001.obj")

	print "{} Vertices".format(len(temp_o.vertices))
	print "{} Faces".format(len(temp_o.face_indices))

	# each vector3f gets converted into 4 pixels
	# x, y, z -> <x0, y0, z0>, <x1, y1, z1>, ...
	WIDTH = 280
	HEIGHT = 280
	POOL_SIZE = 4
	NUM_FRAMES = 270
	BATCH_SIZE = NUM_FRAMES/POOL_SIZE
	WALK = temp_o.walk()
	print "Walk len:", len(WALK)

	# write out the metadata file
	iw = temp_o.inverse_walk
	with open("{}/metadata.ymd".format(outdirectory), "w") as fout:
		for a, b, c in temp_o.face_indices:
			fout.write("{} {} {}\n".format(iw[a], iw[b], iw[c]))

	p = Pool(POOL_SIZE)
	p.map(job, range(0, 270, BATCH_SIZE))

