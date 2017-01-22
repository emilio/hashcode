#![feature(field_init_shorthand)]

use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader, Write};

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
enum Ingredient {
    Tomato,
    Mushroom,
}

struct Pizza {
    field: Vec<Vec<Ingredient>>,
    rows: usize,
    columns: usize,
}

#[derive(Clone, Debug)]
struct Slice {
    x: usize,
    y: usize,
    width: usize,
    height: usize,
    // Just for convenience.
    tomato_count: usize,
    mushroom_count: usize,
}

impl Pizza {
    fn new(field: Vec<Vec<Ingredient>>) -> Self {
        assert!(!field.is_empty());
        let rows = field.len();
        let columns = field[0].len();

        assert!(field.iter().all(|row| row.len() == columns), "Odd-looking field");

        Pizza { field, rows, columns, }
    }

    fn solve(&self, min_per_slice: usize, max_per_slice: usize) -> Vec<Slice> {
        let state = PizzaSolver::new(self, min_per_slice, max_per_slice);
        state.solve()
    }

    fn rows(&self) -> usize {
        self.rows
    }

    fn columns(&self) -> usize {
        self.columns
    }

    fn each<F>(&self, mut f: F)
        where F: FnMut(Ingredient, usize, usize),
    {
        let mut y = 0;
        for row in &self.field {
            let mut x = 0;
            for ingredient in &*row {
                f(*ingredient, x, y);
                x += 1;
            }
            y += 1;
        }
    }

    fn at(&self, x: usize, y: usize) -> Ingredient {
        self.field[y][x]
    }
}

struct PizzaSolver<'a> {
    pizza: &'a Pizza,
    min_of_each_per_slice: usize,
    max_cells_per_slice: usize,
    occupied: Vec<Vec<bool>>,
    slices: Vec<Slice>,
}

// FIXME(emilio): This is slightly unidiomatic to avoid double borrow errors.
//
// The cool part of this function is that at this point we've guaranteed all
// slices are valid, so we can just go ahead and grow.
//
// *shrug*
fn try_expand_slice_towards(max_cells_per_slice: usize,
                            min_of_each_per_slice: usize,
                            update_occupied: bool,
                            care_about_perfect_validity: bool,
                            pizza: &Pizza,
                            occupied: &mut Vec<Vec<bool>>,
                            slice: &mut Slice,
                            direction_x: isize,
                            direction_y: isize)
                            -> bool {
    if slice.width * slice.height >= max_cells_per_slice {
        return false; // already full.
    }

    if (slice.width + direction_x.abs() as usize) *
       (slice.height + direction_y.abs() as usize) > max_cells_per_slice {
        return false; // would overflow
   }

    // Check bounds.
    if direction_x < 0 {
        if slice.x < -direction_x as usize {
            return false;
        }
    } else {
        if slice.x + slice.width + direction_x as usize > pizza.columns() {
            return false;
        }
    }

    if direction_y < 0 {
        if slice.y < -direction_y as usize {
            return false;
        }
    } else {
        if slice.y + slice.height + direction_y as usize > pizza.rows() {
            return false;
        }
    }

    let mut points_to_occupy = vec![];

    // Expand in the x axis.
    let mut new_slice = slice.clone();
    if direction_x < 0 {
        new_slice.x -= -direction_x as usize;
        new_slice.width += -direction_x as usize;
        for x in new_slice.x..slice.x {
            for y in slice.y..slice.y + slice.height {
                if occupied[y][x] {
                    return false;
                }
                points_to_occupy.push((x, y));
            }
        }
    } else {
        new_slice.width += direction_x as usize;
        for x in slice.x + slice.width..slice.x + slice.width + direction_x as usize {
            for y in slice.y..slice.y + slice.height {
                if occupied[y][x] {
                    return false;
                }
                points_to_occupy.push((x, y));
            }
        }
    }

    // Expand in the y axis.
    if direction_y < 0 {
        new_slice.y -= -direction_y as usize;
        new_slice.height += -direction_y as usize;
        for y in new_slice.y..slice.y {
            for x in new_slice.x..new_slice.x + new_slice.width {
                if occupied[y][x] {
                    return false;
                }
                points_to_occupy.push((x, y));
            }
        }
    } else {
        new_slice.height += direction_y as usize;
        for y in slice.y + slice.height..slice.y + slice.height + direction_y as usize {
            for x in new_slice.x..new_slice.x + new_slice.width {
                if occupied[y][x] {
                    return false;
                }
                points_to_occupy.push((x, y));
            }
        }
    }

    // assert_eq!(points_to_occupy.len(),
    //            ((direction_x.abs() + 1) * (direction_y.abs() + 1) - 1) as usize);
    for &(x, y) in &points_to_occupy {
        match pizza.at(x, y) {
            Ingredient::Tomato => new_slice.tomato_count += 1,
            Ingredient::Mushroom => new_slice.mushroom_count += 1,
        }
    }

    if care_about_perfect_validity &&
        new_slice.tomato_count < min_of_each_per_slice &&
        new_slice.mushroom_count < min_of_each_per_slice {
        return false;
    }

    if update_occupied {
        for (x, y) in points_to_occupy {
            occupied[y][x] = true;
        }
    }

    *slice = new_slice;
    true
}

fn slice_valid(solver: &PizzaSolver, slice: &Slice) -> bool {
    slice.tomato_count >= solver.min_of_each_per_slice &&
    slice.mushroom_count >= solver.min_of_each_per_slice &&
    slice.width * slice.height <= solver.max_cells_per_slice
}

impl<'a> PizzaSolver<'a> {
    fn new(pizza: &'a Pizza,
           min_of_each_per_slice: usize,
           max_cells_per_slice: usize) -> Self {
        assert!(max_cells_per_slice >= 1);
        assert!(min_of_each_per_slice <= max_cells_per_slice);
        PizzaSolver {
            pizza,
            min_of_each_per_slice,
            max_cells_per_slice,
            occupied: vec![vec![false; pizza.columns()]; pizza.rows()],
            slices: vec![],
        }
    }

    fn solve(mut self) -> Vec<Slice> {
        self.find_minimal_slices();
        self.expand_slices();
        for s in &self.slices {
            assert!(slice_valid(&self, s));
        }
        self.slices
    }

    fn add_slice(&mut self, slice: Slice) {
        assert_eq!(slice.width * slice.height, slice.tomato_count + slice.mushroom_count);
        for x in slice.x..slice.x + slice.width {
            for y in slice.y..slice.y + slice.height {
                assert!(!self.occupied[y][x]);
                self.occupied[y][x] = true;
            }
        }

        self.slices.push(slice);
    }

    fn occupied(&self, x: usize, y: usize) -> bool {
        self.occupied[y][x]
    }

    fn try_create_slice_at(&mut self, x: usize, y: usize) {
        if self.occupied(x, y) {
            return;
        }

        // Try first to form a slice to the left and top, since we go left to
        // right, top to bottom.
        let is_tomato = self.pizza.at(x, y) == Ingredient::Tomato;
        let base_tomato_count = if is_tomato { 1 } else { 0 };
        let base_mushroom_count = if is_tomato { 0 } else { 1 };

        let base_slice = Slice {
            x: x,
            y: y,
            width: 1,
            height: 1,
            tomato_count: base_tomato_count,
            mushroom_count: base_mushroom_count,
        };

        fn slice_could_grow(solver: &PizzaSolver, slice: &Slice) -> bool {
            slice.width * slice.height < solver.max_cells_per_slice
        }

        macro_rules! try_expand {
            ($slice:expr, $x:expr, $y:expr) => {
                try_expand_slice_towards(self.max_cells_per_slice,
                                         self.min_of_each_per_slice,
                                         false,
                                         false,
                                         &self.pizza,
                                         &mut self.occupied,
                                         $slice,
                                         $x,
                                         $y)
            }
        }
        const SIZES: [(isize, isize); 6] = [
            (-1, 0),
            (0, 1),
            (0, -1),
            (1, 0),
            (-1, -1),
            (1, 1),
        ];

        let mut slice = base_slice.clone();
        'outer: while !slice_valid(self, &slice) {
            for &(x, y) in &SIZES {
                if try_expand!(&mut slice, x, y) {
                    continue 'outer;
                }
            }
            break;
        }

        if slice_valid(self, &slice) {
            self.add_slice(slice);
            return;
        }

        for &(x, y) in &SIZES {
            let mut slice = base_slice.clone();
            while !slice_valid(self, &slice) && try_expand!(&mut slice, x, y) {
                // Keep trying.
            }

            if slice_valid(self, &slice) {
                self.add_slice(slice);
                return;
            }
        }
    }

    fn find_minimal_slices(&mut self) {
        assert!(self.slices.is_empty());
        self.pizza.each(|_ingredient, x, y| {
            self.try_create_slice_at(x, y);
        });
    }

    fn expand_slices(&mut self) {
        macro_rules! try_expand {
            ($slice:expr, $x:expr, $y:expr) => {
                try_expand_slice_towards(self.max_cells_per_slice,
                                         self.min_of_each_per_slice,
                                         true,
                                         true,
                                         &self.pizza,
                                         &mut self.occupied,
                                         $slice,
                                         $x,
                                         $y)
            }
        }

        for slice in &mut self.slices {
            // Same logic, try to expand left then top, since that's how we
            // insert the minimal slices.
            while try_expand!(slice, -1, -1) ||
                  try_expand!(slice, 0, -1) ||
                  try_expand!(slice, -1, 0) ||
                  try_expand!(slice, 1, 1) ||
                  try_expand!(slice, 0, 1) ||
                  try_expand!(slice, 1, 0) {
                // Just try again until we can't.
            }
        }
    }
}

fn main() {
    let filename = env::args().skip(1).next().expect("Expected a filename");

    let f = BufReader::new(File::open(&filename).expect("Couldn't open file"));

    let mut lines = f.lines();
    let (rows, columns, min_per_slice, max_per_slice) = {
        let first_line = lines.next().expect("Expected at least one line").unwrap();
        let mut split = first_line.split(' ');
        let rows = split.next().unwrap().parse::<usize>().unwrap();
        let columns = split.next().unwrap().parse::<usize>().unwrap();
        let min_per_slice = split.next().unwrap().parse::<usize>().unwrap();
        let max_per_slice = split.next().unwrap().parse::<usize>().unwrap();

        (rows, columns, min_per_slice, max_per_slice)
    };

    let mut field = Vec::with_capacity(rows);

    for l in lines {
        let line = l.unwrap();
        let mut row = Vec::with_capacity(columns);
        let mut chars = line.chars();
        for _ in 0..columns {
            row.push(match chars.next().unwrap() {
                'T' => Ingredient::Tomato,
                'M' => Ingredient::Mushroom,
                other => panic!("Unknown ingredient: {:?}", other),
            });
        }
        field.push(row);
    }

    let pizza = Pizza::new(field);
    let slices = pizza.solve(min_per_slice, max_per_slice);

    let mut score = 0;
    println!("{}", slices.len());
    for slice in slices {
        println!("{} {} {} {}",
                 slice.y,
                 slice.x,
                 slice.y + slice.height - 1,
                 slice.x + slice.width - 1);
        score += slice.width * slice.height;
    }

    let _ = writeln!(&mut ::std::io::stderr(), "[{}]: \t{:?}", filename, score);
}
