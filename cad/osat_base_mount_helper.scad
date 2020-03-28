include<osat_cad_param.scad>

x_dist = x_length - glass_mount_hole_x_dist * 2;

difference()
{
  //base
  union()
  {
    cylinder(d = glass_mount_hole_diameter * 2, h = 5, center = false);
    translate([x_dist, 0, 0]) {
      cylinder(d = glass_mount_hole_diameter * 2, h = 5, center = false);
    }
    translate([(x_length - x_dist)/-2, -10, 0]){ cube([x_length, 20, 2], center = false); }
  }
  //holes

  translate([x_dist, 0, 0]) {
      cylinder(d = glass_mount_hole_diameter, h = 25, center = true);
  }
  cylinder(d = glass_mount_hole_diameter, h = 25, center = true);

  //notch
  translate([0, 15, 0]){ cube([glass_mount_hole_diameter, 30, 30], center = true); }
  translate([x_dist, 15, 0]){ cube([glass_mount_hole_diameter, 30, 30], center = true); }

}