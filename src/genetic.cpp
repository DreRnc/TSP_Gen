// representation: vector with the journey (length n. cities with indexes of cities visited)

// fitness: length of the circuit

// mutation: exchange two cells of the vector journey: 1234 --> 3124 (ABCDE -> CBDAE)
// 			if the mapping if A1, B2, C3, D4

// crossover: choose a random crossover point (0 - #cities - 1) but perform the corrective action
// 				to avoid doubling / missing cities