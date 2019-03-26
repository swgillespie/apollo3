#!/usr/bin/env python

from __future__ import print_function
import chess
import optparse
import json
import difflib
import progressbar

parser = optparse.OptionParser()
parser.add_option('-f', '--file',
                  dest='filename', help='Move database, obtained from apollo3 perft',
                  metavar='FILE')

def run_position(position):
    fen = position['fen']
    moves = sorted(position['moves'])
    pychess_board = chess.Board(fen)
    pychess_moves = sorted([x.uci() for x in pychess_board.legal_moves])
    if moves != pychess_moves:
        print('Move generation divergence (fen {0}):'.format(fen))
        print('\n'.join(difflib.unified_diff(pychess_moves, moves)))


def run_all_positions(db):
    bar = progressbar.ProgressBar();
    for position in bar(db):
        run_position(position)

def run(filename):
    with open(filename, 'r+') as f:
        db = json.loads(f.read())
        run_all_positions(db)

def main():
    (ops, args) = parser.parse_args()
    if ops.filename is None:
        print('error: move database file is required')
        exit(1)

    run(ops.filename)
    print('done!')

if __name__ == "__main__":
    main()
