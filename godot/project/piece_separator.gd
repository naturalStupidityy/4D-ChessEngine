extends Node3D

func _ready():
	print("=== LOADING CHESS PIECES ===")
	
	# Just load the file to confirm it works
	var scene = load("res://chess_pieces/all_pieces.glb")
	if scene == null:
		print("ERROR: Could not load all_pieces.glb")
		return
	
	print("SUCCESS: all_pieces.glb loaded!")
	print("This file contains all 6 chess pieces together.")
	print("We'll use this directly in the main game.")
	
	# Create a simple marker file so we know this worked
	var file = FileAccess.open("res://chess_pieces/PIECES_READY.txt", FileAccess.WRITE)
	file.store_string("Chess pieces loaded successfully!")
	file.close()
	
	print("Created marker file: PIECES_READY.txt")
	print("=== DONE ===")
