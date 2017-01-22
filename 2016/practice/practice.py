#!/usr/bin/env python3
import sys

EMPTY = '.'
PAINTED = '#'

def grid(width, height):
  return [[0 for i in range(0, width)] for _ in range(0, height)]

class Point():
  def __init__(self, x, y):
    self.x, self.y = x, y

  def __str__(self):
    return "{} {}".format(self.x, self.y)

class Rect():
  def __init__(self, orig, width, height):
    self.orig = orig
    self.width = width
    self.height = height

  def __str__(self):
    return "RECT {} {} {}".format(self.orig, self.width, self.height)

class HLine(Rect):
  def __init__(self, orig, width):
    super().__init__(orig, width, 1)

class VLine(Rect):
  def __init__(self, orig, height):
    super().__init__(orig, 1, height)

class Square(Rect):
  def __init__(self, orig, width):
    super().__init__(orig, width, width)

class Command():
  pass

class PaintLineCommand(Command):
  def __init__(self, fr, to):
    self.fr, self.to = fr, to

  def __str__(self):
    return "PAINT_LINE {} {}".format(self.fr, self.to)

class PaintSquareCommand(Command):
  def __init__(self, center, side):
    self.center, self.side = center, side

  def __str__(self):
    return "PAINT_SQUARE {} {}".format(center, side)

def fill_with_hlines(image, painted):
  commands = []
  assert len(image) == len(painted)
  assert len(image[0]) == len(painted[0])
  for row, r in enumerate(image):
    for col, _ in enumerate(r):
      if image[row][col] == PAINTED:
        if not painted[row][col]:
          current = col
          while current < len(r):
            if image[row][current] == PAINTED and not painted[row][current]:
              current += 1
            else:
              break
          horizontal_line_len = current - col
          current = row
          while current < len(image):
            if image[current][col] == PAINTED and not painted[current][col]:
              current += 1
            else:
              break
          vertical_line_len = current - row
          if horizontal_line_len > vertical_line_len:
            for i in range(0, horizontal_line_len):
              painted[row][col + i] = 1
            commands.append(HLine(Point(row, col), horizontal_line_len))
          else:
            for i in range(0, vertical_line_len):
              painted[row + i][col] = 1
            commands.append(VLine(Point(row, col), vertical_line_len))

  return commands, painted

def try_find_worthy_square_from(image, painted, row, col):
  best_size = 0
  current_size = 0
  height, width = len(painted), len(painted[0])

  # TODO: Tweacking this we might get better results
  min_worthy_size = round((height + width) / 80)

  assert not painted[row][col]

  while True:
    dim = 2 * current_size + 1

    if row + dim > height or col + dim > width:
      break

    for i in range(row, row + dim):
      for j in range(col, col + dim):
        if image[i][j] == EMPTY:
          if best_size < min_worthy_size:
            return None
          return Square(Point(row, col), best_size * 2 + 1)
    best_size = current_size
    current_size += 1

  if best_size < min_worthy_size:
    return None

  return Square(Point(row, col), best_size * 2 + 1)



def find_worthy_squares(image):
  painted = grid(len(image[0]), len(image))
  commands = []
  for row, r in enumerate(image):
    for col, _ in enumerate(r):
      if image[row][col] == PAINTED and not painted[row][col]:
          square = try_find_worthy_square_from(image, painted, row, col)
          if square:
            size = square.width
            for i in range(0, size):
              for j in range(0, size):
                painted[row + i][col + j] = 1
            commands.append(square)

  return commands, painted

def main():
  assert len(sys.argv) > 1
  out = []
  with open(sys.argv[1]) as f:
    height, width = map(int, f.readline().strip().split(' '))
    for line in f.readlines():
      out.append(list(line.strip()))

  squares, painted = find_worthy_squares(out)
  # print(len(squares))

  lines, painted = fill_with_hlines(out, painted)

  squares.extend(lines)
  print(len(squares))
  # We should paint the commands here, but I haven't been able to beat the C
  # program in any single case

if __name__ == '__main__':
  main()
