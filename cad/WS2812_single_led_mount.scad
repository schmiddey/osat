
height = 5;

$fn = 100;



difference()
{
  //base body
  union()
  {
    cylinder(d = 12,5, h = height, center = false);
    translate([8, 0, 0])  cylinder(d=6, h=height, center=false);
    translate([-8, 0, 0]) cylinder(d=6, h=height, center=false);
  }

  //Hole for LED light
  cylinder(d = 5.0, h = 50, center = true);
  //Hole for LED body (9.5mm)
  translate([0, 0, 1])  cylinder(d = 9.5, h = 50, center = false);
  //mounting holes M3
  #translate([8, 0, 0])  cylinder(d = 3.2, h = 50, center = true);
  #translate([-8, 0, 0]) cylinder(d = 3.2, h = 50, center = true);

  translate([-3.5, 0, 2.5]) cube([7, 10, 10], center = false);

}

