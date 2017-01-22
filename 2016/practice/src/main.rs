#![feature(field_init_shorthand)]

use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};

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

    fn total_ingredients(&self) -> usize {
        self.rows() * self.columns()
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

    fn less_present_ingredient(&self) -> Ingredient {
        let mut tomato_count = 0;
        self.each(|ingredient, _, _| {
            if ingredient == Ingredient::Tomato {
                tomato_count += 1;
            }
        });

        if self.total_ingredients() - tomato_count > tomato_count  {
            Ingredient::Tomato
        } else {
            Ingredient::Mushroom
        }
    }
}

struct PizzaSolver<'a> {
    pizza: &'a Pizza,
    min_of_each_per_slice: usize,
    max_cells_per_slice: usize,
    occupied: Vec<Vec<bool>>,
    slices: Vec<Slice>,
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
        self.slices
    }

    fn add_slice(&mut self, slice: Slice) {
        for x in slice.x..slice.x + slice.width {
            for y in slice.y..slice.y + slice.height {
                assert!(!self.occupied[x][y]);
                self.occupied[x][y] = true;
            }
        }

        self.slices.push(slice);
    }

    // FIXME(emilio): We probably want to make this O(1) since we check this _a
    // lot_.
    fn occupied(&self, x: usize, y: usize) -> bool {
        self.occupied[x][y]
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

        fn slice_valid(solver: &PizzaSolver, slice: &Slice) -> bool {
            slice.tomato_count >= solver.min_of_each_per_slice &&
            slice.mushroom_count >= solver.min_of_each_per_slice &&
            slice.width * slice.height <= solver.max_cells_per_slice
        }

        fn slice_could_grow(solver: &PizzaSolver, slice: &Slice) -> bool {
            slice.width * slice.height <= solver.max_cells_per_slice
        }

        let mut slice = base_slice.clone();

        // Try to grow to the top.
        while !slice_valid(self, &slice) &&
            slice_could_grow(self, &slice) &&
            slice.y > 0 &&
            !self.occupied(slice.x, slice.y - 1) {
            slice.y -= 1;
            slice.height += 1;
            match self.pizza.at(slice.x, slice.y) {
                Ingredient::Tomato => slice.tomato_count += 1,
                Ingredient::Mushroom => slice.mushroom_count += 1,
            }
        }

        // TODO: We could keep going and compare all the valid slices?
        if slice_valid(self, &slice) {
            self.add_slice(slice);
            return;
        }

        slice = base_slice.clone();

        // Try to grow to the left.
        while !slice_valid(self, &slice) &&
            slice_could_grow(self, &slice) &&
            slice.x > 0 &&
            !self.occupied(slice.x - 1, slice.y) {
            slice.x -= 1;
            slice.width += 1;
            match self.pizza.at(slice.x, slice.y) {
                Ingredient::Tomato => slice.tomato_count += 1,
                Ingredient::Mushroom => slice.mushroom_count += 1,
            }
        }

        if slice_valid(self, &slice) {
            self.add_slice(slice);
            return;
        }

        // FIXME: Still smelling.
        slice = base_slice.clone();

        // Try to grow to the right.
        while !slice_valid(self, &slice) &&
            slice_could_grow(self, &slice) &&
            slice.x + slice.width < self.pizza.columns() &&
            !self.occupied(slice.x + slice.width, slice.y) {
            slice.width += 1;
            match self.pizza.at(slice.x + slice.width - 1, slice.y) {
                Ingredient::Tomato => slice.tomato_count += 1,
                Ingredient::Mushroom => slice.mushroom_count += 1,
            }
        }

        if slice_valid(self, &slice) {
            self.add_slice(slice);
            return;
        }

        slice = base_slice.clone();

        // Try to grow to the bottom.
        while !slice_valid(self, &slice) &&
            slice_could_grow(self, &slice) &&
            slice.y + slice.height < self.pizza.rows() &&
            !self.occupied(slice.x, slice.y + slice.height) {
            slice.height += 1;
            match self.pizza.at(slice.x, slice.y + slice.height - 1) {
                Ingredient::Tomato => slice.tomato_count += 1,
                Ingredient::Mushroom => slice.mushroom_count += 1,
            }
        }

        if slice_valid(self, &slice) {
            self.add_slice(slice);
            return;
        }

        // TODO: We could try to find squares and such here, though it slightly
        // complicates the logic there.
        //
        // Nothing super-important though.
    }

    fn find_minimal_slices(&mut self) {
        assert!(self.slices.is_empty());
        let less_present_ingredient = self.pizza.less_present_ingredient();
        self.pizza.each(|ingredient, x, y| {
            if ingredient == less_present_ingredient {
                self.try_create_slice_at(x, y);
            }
        });
    }

    fn expand_slices(&mut self) {
        // TODO
        // for slice in &mut slices {
        //     // Same logic, try to expand left then top, since that's how we
        //     // insert the minimal slices.
        //
        // }
    }
}

fn main() {
    let filename = env::args().skip(1).next().expect("Expected a filename");

    let f = BufReader::new(File::open(filename).expect("Couldn't open file"));

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
    println!("{:?}", slices);
}
