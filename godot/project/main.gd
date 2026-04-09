extends Node3D

const MINI_SQUARE_SIZE = 1.0
const MINI_BOARD_SPACING = 6.0
const PIECE_SCALE = 0.8
const FULL_PIECE_SCALE = 3.5

enum Mode {OVERVIEW, FOCUS}
enum PieceType {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN}

var current_mode = Mode.OVERVIEW
var focus_w: int = 0
var focus_z: int = 0

var mini_boards: Array = []
var full_board: Node3D = null
var camera: Camera3D = null

var pieces = {}
var visible_pieces = []

# Movement system
var selected_piece_key = null
var current_turn = Color.WHITE

func _ready():
	print("=== 4D CHESS - FIXED VERSION ===")
	setup_camera()
	setup_environment()
	setup_lighting()
	create_overview_boards()
	setup_pieces()
	show_overview_pieces()
	print("Controls: CLICK mini-board to zoom in | ESC to zoom out")

func setup_camera():
	camera = Camera3D.new()
	camera.name = "Camera"
	add_child(camera)
	setup_camera_overview()

func setup_camera_overview():
	var center = 1.5 * MINI_BOARD_SPACING
	camera.position = Vector3(center, 35, center)
	camera.rotation_degrees = Vector3(-90, 0, 0)
	camera.fov = 40

func setup_camera_focus(w: int, z: int):
	var board_center_x = w * MINI_BOARD_SPACING + 1.5
	var board_center_z = z * MINI_BOARD_SPACING + 1.5
	var cam_pos = Vector3(board_center_x, 25, board_center_z + 20)
	
	var tween = create_tween()
	tween.tween_property(camera, "position", cam_pos, 0.4)
	tween.parallel().tween_property(camera, "fov", 50, 0.4)
	tween.tween_callback(func():
		camera.look_at(Vector3(board_center_x, 0, board_center_z), Vector3.UP)
	)

func setup_environment():
	var env = WorldEnvironment.new()
	var er = Environment.new()
	er.background_mode = Environment.BG_COLOR
	er.background_color = Color(0.1, 0.1, 0.12)
	er.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	er.ambient_light_color = Color(0.5, 0.5, 0.55)
	er.ambient_light_energy = 0.8
	env.environment = er
	add_child(env)

func setup_lighting():
	var light = DirectionalLight3D.new()
	light.rotation_degrees = Vector3(-60, 45, 0)
	light.light_color = Color(1, 0.98, 0.95)
	light.light_energy = 1.3
	light.shadow_enabled = true
	add_child(light)

func create_overview_boards():
	for w in range(4):
		mini_boards.append([])
		for z in range(4):
			var mini = create_mini_board(w, z)
			mini.position = Vector3(w * MINI_BOARD_SPACING, 0, z * MINI_BOARD_SPACING)
			add_child(mini)
			mini_boards[w].append(mini)

func create_mini_board(w: int, z: int) -> Node3D:
	var board = Node3D.new()
	
	# Add base with collision
	var base_body = StaticBody3D.new()
	var base_col = CollisionShape3D.new()
	var base_box = BoxShape3D.new()
	base_box.size = Vector3(5, 0.3, 5)
	base_col.shape = base_box
	base_col.position.y = -0.15
	base_body.add_child(base_col)
	
	var base_mesh = MeshInstance3D.new()
	var bm = BoxMesh.new()
	bm.size = Vector3(5, 0.3, 5)
	base_mesh.mesh = bm
	var mat = StandardMaterial3D.new()
	mat.albedo_color = Color(0.25, 0.15, 0.08)
	base_mesh.material_override = mat
	base_mesh.position.y = -0.15
	base_body.add_child(base_mesh)
	
	board.add_child(base_body)
	
	for x in range(4):
		for y in range(4):
			var sq = create_mini_square(x, y)
			board.add_child(sq)
	
	var label = Label3D.new()
	label.text = "W%d,Z%d" % [w, z]
	label.position = Vector3(0, 2.5, 0)
	label.font_size = 100
	label.modulate = Color(1, 1, 1)
	label.billboard = BaseMaterial3D.BILLBOARD_ENABLED
	board.add_child(label)
	
	return board

func create_mini_square(x: int, y: int) -> StaticBody3D:
	var body = StaticBody3D.new()
	body.name = "mini_square_%d_%d" % [x, y]
	
	var col = CollisionShape3D.new()
	var box = BoxShape3D.new()
	box.size = Vector3(MINI_SQUARE_SIZE - 0.05, 0.1, MINI_SQUARE_SIZE - 0.05)
	col.shape = box
	body.add_child(col)
	
	var mesh = MeshInstance3D.new()
	var sm = BoxMesh.new()
	sm.size = Vector3(MINI_SQUARE_SIZE - 0.05, 0.1, MINI_SQUARE_SIZE - 0.05)
	mesh.mesh = sm
	
	var m = StandardMaterial3D.new()
	if (x + y) % 2 == 0:
		m.albedo_color = Color(0.95, 0.92, 0.85)
	else:
		m.albedo_color = Color(0.55, 0.35, 0.2)
	mesh.material_override = m
	body.add_child(mesh)
	
	var offset = 1.5 * MINI_SQUARE_SIZE
	body.position = Vector3(x * MINI_SQUARE_SIZE - offset, 0.05, y * MINI_SQUARE_SIZE - offset)
	
	return body

func enter_focus_mode(w: int, z: int):
	current_mode = Mode.FOCUS
	focus_w = w
	focus_z = z
	
	for row in mini_boards:
		for mini in row:
			mini.visible = false
	
	clear_visible_pieces()
	create_full_board(w, z)
	setup_camera_focus(w, z)
	show_pieces_for_layer(w, z, true)

func create_full_board(w: int, z: int):
	if full_board != null:
		full_board.queue_free()
	
	full_board = Node3D.new()
	full_board.name = "FullBoard"
	
	var board_origin_x = w * MINI_BOARD_SPACING
	var board_origin_z = z * MINI_BOARD_SPACING
	full_board.position = Vector3(board_origin_x, 0, board_origin_z)
	
	# Base with collision
	var base_body = StaticBody3D.new()
	var base_col = CollisionShape3D.new()
	var base_box = BoxShape3D.new()
	base_box.size = Vector3(18, 1, 18)
	base_col.shape = base_box
	base_col.position = Vector3(1.5, -0.5, 1.5)
	base_body.add_child(base_col)
	
	var base_mesh = MeshInstance3D.new()
	var bm = BoxMesh.new()
	bm.size = Vector3(18, 1, 18)
	base_mesh.mesh = bm
	var mat = StandardMaterial3D.new()
	mat.albedo_color = Color(0.3, 0.2, 0.1)
	base_mesh.material_override = mat
	base_mesh.position = Vector3(1.5, -0.5, 1.5)
	base_body.add_child(base_mesh)
	
	full_board.add_child(base_body)
	
	for x in range(4):
		for y in range(4):
			var sq = create_full_square(x, y)
			full_board.add_child(sq)
	
	var label = Label3D.new()
	label.text = "LAYER W=%d, Z=%d" % [w, z]
	label.position = Vector3(1.5, 6, 8)
	label.font_size = 100
	label.modulate = Color(1, 0.9, 0.7)
	label.billboard = BaseMaterial3D.BILLBOARD_ENABLED
	full_board.add_child(label)
	
	add_child(full_board)

func create_full_square(x: int, y: int) -> StaticBody3D:
	var body = StaticBody3D.new()
	body.name = "square_%d_%d" % [x, y]
	
	var pos_x = x * 4.5 - 6.75 + 1.5
	var pos_z = y * 4.5 - 6.75 + 1.5
	body.position = Vector3(pos_x, 0.15, pos_z)
	
	var col = CollisionShape3D.new()
	var box = BoxShape3D.new()
	box.size = Vector3(4, 0.3, 4)
	col.shape = box
	body.add_child(col)
	
	var mesh = MeshInstance3D.new()
	var sm = BoxMesh.new()
	sm.size = Vector3(4, 0.3, 4)
	mesh.mesh = sm
	
	var m = StandardMaterial3D.new()
	if (x + y) % 2 == 0:
		m.albedo_color = Color(0.95, 0.92, 0.85)
	else:
		m.albedo_color = Color(0.6, 0.4, 0.25)
	mesh.material_override = m
	body.add_child(mesh)
	
	return body

# ==================== MOUSE INPUT ====================

func _input(event):
	# Handle ESC first, before anything else
	if event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
		if current_mode == Mode.FOCUS:
			return_to_overview()
			get_viewport().set_input_as_handled()  # Prevent ESC from closing window
		return
	
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		handle_mouse_click()

func handle_mouse_click():
	var mouse_pos = get_viewport().get_mouse_position()
	var from = camera.project_ray_origin(mouse_pos)
	var to = from + camera.project_ray_normal(mouse_pos) * 1000
	
	var space_state = get_world_3d().direct_space_state
	var query = PhysicsRayQueryParameters3D.new()
	query.from = from
	query.to = to
	query.collide_with_bodies = true
	query.collide_with_areas = true
	
	var result = space_state.intersect_ray(query)
	
	if result.is_empty():
		print("HIT NOTHING")
		return
	
	print("HIT: %s" % result.collider.name)
	
	var clicked_pos = result.position
	
	if current_mode == Mode.OVERVIEW:
		check_overview_click(clicked_pos)
	else:
		check_focus_click(clicked_pos)

func check_overview_click(world_pos: Vector3):
	var w = int(round(world_pos.x / MINI_BOARD_SPACING))
	var z = int(round(world_pos.z / MINI_BOARD_SPACING))
	w = clamp(w, 0, 3)
	z = clamp(z, 0, 3)
	print("Entering board W=%d, Z=%d" % [w, z])
	enter_focus_mode(w, z)

func check_focus_click(world_pos: Vector3):
	var local_x = world_pos.x - full_board.position.x
	var local_z = world_pos.z - full_board.position.z
	
	var x = int(floor((local_x + 6.75) / 4.5))
	var y = int(floor((local_z + 6.75) / 4.5))
	
	x = clamp(x, 0, 3)
	y = clamp(y, 0, 3)
	
	print("Square: X=%d, Y=%d" % [x, y])
	handle_square_click(x, y)

func handle_square_click(x: int, y: int):
	var clicked_key = "%d,%d,%d,%d" % [focus_w, x, y, focus_z]
	print("Key: %s" % clicked_key)
	
	if pieces.has(clicked_key):
		var piece = pieces[clicked_key]
		var name = get_piece_name(piece.type)
		var color = "White" if piece.color == Color.WHITE else "Black"
		print("Piece: %s (%s)" % [name, color])
		
		if piece.color == current_turn:
			select_piece(clicked_key)
			return
		else:
			print("Wrong color")
	
	if selected_piece_key != null:
		if is_valid_move(selected_piece_key, clicked_key):
			execute_move(selected_piece_key, clicked_key)
		else:
			print("Invalid move")
			deselect_piece()
	else:
		print("No piece selected")

func select_piece(key: String):
	deselect_piece()
	selected_piece_key = key
	var piece = pieces[key]
	print("SELECTED: %s" % get_piece_name(piece.type))
	highlight_square(piece.x, piece.y, Color.GREEN)

func deselect_piece():
	selected_piece_key = null
	# FIX: Check if full_board exists before calling get_children()
	if full_board != null:
		for child in full_board.get_children():
			if child.name.begins_with("highlight_"):
				child.queue_free()

func highlight_square(x: int, y: int, color: Color):
	# FIX: Check if full_board exists
	if full_board == null:
		return
		
	var highlight = MeshInstance3D.new()
	highlight.name = "highlight_%d_%d" % [x, y]
	
	var cyl = CylinderMesh.new()
	cyl.top_radius = 1.8
	cyl.bottom_radius = 1.8
	cyl.height = 0.05
	highlight.mesh = cyl
	
	var mat = StandardMaterial3D.new()
	mat.albedo_color = color
	mat.emission_enabled = true
	mat.emission = color
	mat.emission_energy = 2.0
	highlight.material_override = mat
	
	var pos_x = x * 4.5 - 6.75 + 1.5
	var pos_z = y * 4.5 - 6.75 + 1.5
	highlight.position = Vector3(pos_x, 0.1, pos_z)
	
	full_board.add_child(highlight)

# ==================== MOVEMENT ====================

func is_valid_move(from_key: String, to_key: String) -> bool:
	if not pieces.has(from_key):
		return false
	
	var from_piece = pieces[from_key]
	var to_coords = parse_key(to_key)
	var from_coords = parse_key(from_key)
	
	if pieces.has(to_key) and pieces[to_key].color == from_piece.color:
		return false
	
	match from_piece.type:
		PieceType.PAWN:
			return is_valid_pawn_move(from_coords, to_coords, from_piece.color)
		PieceType.ROOK:
			return is_valid_rook_move(from_coords, to_coords)
		PieceType.KNIGHT:
			return is_valid_knight_move(from_coords, to_coords)
		PieceType.BISHOP:
			return is_valid_bishop_move(from_coords, to_coords)
		PieceType.QUEEN:
			return is_valid_queen_move(from_coords, to_coords)
		PieceType.KING:
			return is_valid_king_move(from_coords, to_coords)
	return false

func parse_key(key: String) -> Dictionary:
	var parts = key.split(",")
	return {"w": int(parts[0]), "x": int(parts[1]), "y": int(parts[2]), "z": int(parts[3])}

func is_valid_pawn_move(from: Dictionary, to: Dictionary, color: Color) -> bool:
	var direction = -1 if color == Color.WHITE else 1
	if from.w != to.w or from.z != to.z:
		return false
	
	var dx = to.x - from.x
	var dy = to.y - from.y
	
	if dx == 0 and dy == direction:
		var target_key = "%d,%d,%d,%d" % [to.w, to.x, to.y, to.z]
		return not pieces.has(target_key)
	
	if abs(dx) == 1 and dy == direction:
		var target_key = "%d,%d,%d,%d" % [to.w, to.x, to.y, to.z]
		return pieces.has(target_key) and pieces[target_key].color != color
	
	return false

func is_valid_rook_move(from: Dictionary, to: Dictionary) -> bool:
	var axes = 0
	if abs(to.w - from.w) > 0: axes += 1
	if abs(to.x - from.x) > 0: axes += 1
	if abs(to.y - from.y) > 0: axes += 1
	if abs(to.z - from.z) > 0: axes += 1
	return axes == 1

func is_valid_knight_move(from: Dictionary, to: Dictionary) -> bool:
	var diffs = [abs(to.w - from.w), abs(to.x - from.x), abs(to.y - from.y), abs(to.z - from.z)]
	diffs.sort()
	return diffs == [0, 0, 1, 2]

func is_valid_bishop_move(from: Dictionary, to: Dictionary) -> bool:
	var non_zero = []
	var diffs = [abs(to.w - from.w), abs(to.x - from.x), abs(to.y - from.y), abs(to.z - from.z)]
	for d in diffs:
		if d > 0: non_zero.append(d)
	return non_zero.size() == 2 and non_zero[0] == non_zero[1]

func is_valid_queen_move(from: Dictionary, to: Dictionary) -> bool:
	return is_valid_rook_move(from, to) or is_valid_bishop_move(from, to)

func is_valid_king_move(from: Dictionary, to: Dictionary) -> bool:
	var total = abs(to.w - from.w) + abs(to.x - from.x) + abs(to.y - from.y) + abs(to.z - from.z)
	return total == 1

func execute_move(from_key: String, to_key: String):
	var piece = pieces[from_key]
	var to_coords = parse_key(to_key)
	
	if pieces.has(to_key):
		print("CAPTURED %s!" % get_piece_name(pieces[to_key].type))
		pieces.erase(to_key)
	
	pieces.erase(from_key)
	piece.w = to_coords.w
	piece.x = to_coords.x
	piece.y = to_coords.y
	piece.z = to_coords.z
	pieces[to_key] = piece
	
	print("MOVED to %s" % to_key)
	
	current_turn = Color.BLACK if current_turn == Color.WHITE else Color.WHITE
	print("Turn: %s" % ("BLACK" if current_turn == Color.BLACK else "WHITE"))
	
	deselect_piece()
	show_pieces_for_layer(focus_w, focus_z, true)

func get_piece_name(type: int) -> String:
	match type:
		PieceType.KING: return "King"
		PieceType.QUEEN: return "Queen"
		PieceType.ROOK: return "Rook"
		PieceType.BISHOP: return "Bishop"
		PieceType.KNIGHT: return "Knight"
		PieceType.PAWN: return "Pawn"
	return "Unknown"

func return_to_overview():
	current_mode = Mode.OVERVIEW
	if full_board != null:
		full_board.queue_free()
		full_board = null
	clear_visible_pieces()
	deselect_piece()
	for row in mini_boards:
		for mini in row:
			mini.visible = true
	show_overview_pieces()
	setup_camera_overview()
	print("Returned to overview")

# ==================== PIECES ====================

func setup_pieces():
	place_piece(0, 0, 0, 0, PieceType.ROOK, Color.WHITE)
	place_piece(0, 1, 0, 0, PieceType.KNIGHT, Color.WHITE)
	place_piece(0, 2, 0, 0, PieceType.BISHOP, Color.WHITE)
	place_piece(0, 3, 0, 0, PieceType.QUEEN, Color.WHITE)
	place_piece(0, 0, 1, 0, PieceType.KING, Color.WHITE)
	place_piece(0, 1, 1, 0, PieceType.PAWN, Color.WHITE)
	place_piece(0, 2, 1, 0, PieceType.PAWN, Color.WHITE)
	place_piece(0, 3, 1, 0, PieceType.PAWN, Color.WHITE)
	place_piece(0, 0, 2, 0, PieceType.PAWN, Color.WHITE)
	place_piece(0, 1, 2, 0, PieceType.PAWN, Color.WHITE)
	place_piece(0, 2, 2, 0, PieceType.PAWN, Color.WHITE)
	place_piece(0, 3, 2, 0, PieceType.PAWN, Color.WHITE)
	
	place_piece(3, 0, 3, 3, PieceType.ROOK, Color.BLACK)
	place_piece(3, 1, 3, 3, PieceType.KNIGHT, Color.BLACK)
	place_piece(3, 2, 3, 3, PieceType.BISHOP, Color.BLACK)
	place_piece(3, 3, 3, 3, PieceType.QUEEN, Color.BLACK)
	place_piece(3, 0, 2, 3, PieceType.KING, Color.BLACK)
	place_piece(3, 1, 2, 3, PieceType.PAWN, Color.BLACK)
	place_piece(3, 2, 2, 3, PieceType.PAWN, Color.BLACK)
	place_piece(3, 3, 2, 3, PieceType.PAWN, Color.BLACK)
	place_piece(3, 0, 1, 3, PieceType.PAWN, Color.BLACK)
	place_piece(3, 1, 1, 3, PieceType.PAWN, Color.BLACK)
	place_piece(3, 2, 1, 3, PieceType.PAWN, Color.BLACK)
	place_piece(3, 3, 1, 3, PieceType.PAWN, Color.BLACK)

func place_piece(w, x, y, z, type, color):
	var key = "%d,%d,%d,%d" % [w, x, y, z]
	pieces[key] = {"type": type, "color": color, "w": w, "x": x, "y": y, "z": z}

func show_overview_pieces():
	for key in pieces:
		var p = pieces[key]
		var piece_node = create_piece_mesh(p.type, p.color, false)
		var pos_x = p.w * MINI_BOARD_SPACING + p.x * MINI_SQUARE_SIZE - 1.5
		var pos_z = p.z * MINI_BOARD_SPACING + p.y * MINI_SQUARE_SIZE - 1.5
		piece_node.position = Vector3(pos_x, 0.5, pos_z)
		add_child(piece_node)
		visible_pieces.append(piece_node)

func show_pieces_for_layer(w, z, is_full_size):
	clear_visible_pieces()
	for key in pieces:
		var p = pieces[key]
		if p.w == w and p.z == z:
			var piece_node = create_piece_mesh(p.type, p.color, is_full_size)
			if is_full_size:
				var pos_x = p.x * 4.5 - 6.75 + 1.5
				var pos_z = p.y * 4.5 - 6.75 + 1.5
				piece_node.position = Vector3(pos_x, 1.5, pos_z)
				full_board.add_child(piece_node)
			else:
				var pos_x = w * MINI_BOARD_SPACING + p.x * MINI_SQUARE_SIZE - 1.5
				var pos_z = z * MINI_BOARD_SPACING + p.y * MINI_SQUARE_SIZE - 1.5
				piece_node.position = Vector3(pos_x, 0.5, pos_z)
				add_child(piece_node)
			visible_pieces.append(piece_node)

func clear_visible_pieces():
	for piece in visible_pieces:
		if piece != null:
			piece.queue_free()
	visible_pieces.clear()

func create_piece_mesh(type, color, is_full_size):
	var root = Node3D.new()
	var mesh_instance = MeshInstance3D.new()
	var material = StandardMaterial3D.new()
	material.albedo_color = Color(0.95, 0.95, 0.9) if color == Color.WHITE else Color(0.15, 0.15, 0.15)
	var scale = FULL_PIECE_SCALE if is_full_size else PIECE_SCALE
	
	match type:
		PieceType.KING:
			var cyl = CylinderMesh.new()
			cyl.top_radius = 0.2; cyl.bottom_radius = 0.25; cyl.height = 0.8
			mesh_instance.mesh = cyl
			var crown = MeshInstance3D.new()
			var box = BoxMesh.new()
			box.size = Vector3(0.15, 0.15, 0.15)
			crown.mesh = box; crown.position.y = 0.5; crown.material_override = material
			mesh_instance.add_child(crown)
		PieceType.QUEEN:
			var cyl = CylinderMesh.new()
			cyl.top_radius = 0.2; cyl.bottom_radius = 0.25; cyl.height = 0.7
			mesh_instance.mesh = cyl
			var sphere = MeshInstance3D.new()
			var sph = SphereMesh.new()
			sph.radius = 0.12; sph.height = 0.24
			sphere.mesh = sph; sphere.position.y = 0.42; sphere.material_override = material
			mesh_instance.add_child(sphere)
		PieceType.ROOK:
			var cyl = CylinderMesh.new()
			cyl.top_radius = 0.22; cyl.bottom_radius = 0.22; cyl.height = 0.6
			mesh_instance.mesh = cyl
			var top = MeshInstance3D.new()
			var box = BoxMesh.new()
			box.size = Vector3(0.44, 0.1, 0.44)
			top.mesh = box; top.position.y = 0.35; top.material_override = material
			mesh_instance.add_child(top)
		PieceType.BISHOP:
			var cone = CylinderMesh.new()
			cone.top_radius = 0.1; cone.bottom_radius = 0.25; cone.height = 0.65
			mesh_instance.mesh = cone
			var slit = MeshInstance3D.new()
			var box = BoxMesh.new()
			box.size = Vector3(0.05, 0.15, 0.2)
			slit.mesh = box; slit.position.y = 0.4; slit.material_override = material
			mesh_instance.add_child(slit)
		PieceType.KNIGHT:
			var box = BoxMesh.new()
			box.size = Vector3(0.3, 0.5, 0.4)
			mesh_instance.mesh = box
			var head = MeshInstance3D.new()
			var head_box = BoxMesh.new()
			head_box.size = Vector3(0.25, 0.2, 0.35)
			head.mesh = head_box; head.position = Vector3(0, 0.2, 0.1); head.material_override = material
			mesh_instance.add_child(head)
		PieceType.PAWN:
			var sphere = SphereMesh.new()
			sphere.radius = 0.2; sphere.height = 0.4
			mesh_instance.mesh = sphere
			var base = MeshInstance3D.new()
			var cyl = CylinderMesh.new()
			cyl.top_radius = 0.15; cyl.bottom_radius = 0.2; cyl.height = 0.3
			base.mesh = cyl; base.position.y = -0.2; base.material_override = material
			mesh_instance.add_child(base)
	
	mesh_instance.material_override = material
	mesh_instance.scale = Vector3(scale, scale, scale)
	root.add_child(mesh_instance)
	return root
