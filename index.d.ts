export interface DominoModuleOptions {
  print?: (...args: string[]) => void;
  printErr?: (...args: string[]) => void;
  locateFile?: (path: string, prefix: string) => string;
  [key: string]: unknown;
}

export declare const wasmUrl: string;

export interface DominoAiModule extends EmscriptenModule {
  wasmMemory: WebAssembly.Memory & { buffer: SharedArrayBuffer };

  _init_fact(): void;
  _get_fallback_ptr(): number;
  _get_fallback(): number;
  _reset_fallback(): void;

  _get_LEFT(): number;
  _get_RIGHT(): number;
  _get_PERFECT_PICK(): number;
  _get_IMPERFECT_PICK(): number;
  _get_PASS(): number;

  _alloc_game(): number;
  _alloc_move(): number;
  _alloc_max_move_arr(): number;
  _alloc_int(): number;
  _deref_int(ptr: number): number;
  _alloc_float(): number;
  _deref_float(ptr: number): number;

  _get_number_of_players(): number;
  _get_pips(): number;
  _get_current_player(): number;

  _init_game(gamePtr: number): void;
  _print_game(gamePtr: number): void;
  _get_hands(gamePtr: number): number;
  _set_hand_size(gamePtr: number, player: number, size: number): void;
  _set_turn(gamePtr: number, player: number): void;
  _get_turn(gamePtr: number): number;
  _get_snake(gamePtr: number): number;
  _collapse_piece(
    player: number,
    handsPtr: number,
    left: number,
    right: number,
  ): void;
  _absent_piece(
    player: number,
    handsPtr: number,
    left: number,
    right: number,
  ): void;
  _emit_collapse(handsPtr: number): void;

  _get_playing_moves(
    gamePtr: number,
    playingMovesArrPtr: number,
    numberOfPlayingMovesPtr: number,
    cantPassPtr: number,
  ): void;
  _get_perfect_picking_moves(
    gamePtr: number,
    pickingMovesArrPtr: number,
    numberOfPickingMovesPtr: number,
  ): void;
  _get_playable_perfect_picking_moves(
    gamePtr: number,
    playablePickingMovesArrPtr: number,
    numberOfPlayablePickingMovesPtr: number,
  ): void;

  _populate_move_from_components(
    movePtr: number,
    type: number,
    left: number,
    right: number,
  ): void;
  _populate_imperfect_picking_move(movePtr: number, amount: number): void;
  _populate_move_by_ai(
    gamePtr: number,
    movePtr: number,
    playingMovesArrPtr: number,
    numberOfPlayingMoves: number,
    depth: number,
    scorePtr: number,
    numberOfExploredNodesPtr: number,
  ): void;

  _play_move_by_pointer(gamePtr: number, movePtr: number): void;
  _pass(gamePtr: number): void;
  _perfect_pick_by_pointer(gamePtr: number, movePtr: number): void;
  _imperfect_pick_by_pointer(gamePtr: number, movePtr: number): void;

  _get_left_of_move(movePtr: number): number;
  _get_right_of_move(movePtr: number): number;
  _get_type_of_move(movePtr: number): number;

  _print_playing_moves(
    playingMovesArrPtr: number,
    numberOfPlayingMovesPtr: number,
  ): void;
  _print_picking_moves(
    playingMovesArrPtr: number,
    numberOfPlayingMovesPtr: number,
  ): void;
  _pick_unplayable_domino_probability_from_moves(
    gamePtr: number,
    playingMovesArrPtr: number,
    numberOfPlayingMovesPtr: number,
  ): void;
  _pass_probability_from_num_moves(
    gamePtr: number,
    numberOfPlayingMoves: number,
  ): number;
}

export declare function createModule(
  options?: DominoModuleOptions,
): Promise<DominoAiModule>;

export default createModule;
