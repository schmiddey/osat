include<osat_cad_param.scad>

union()
{
  difference()
  {
    cube([piezo_mount_width + wall_thickness*2, y_length_piezo_fix, wall_thickness * 2], center = false);
    translate([wall_thickness, 0, 0]) cube([piezo_mount_width, y_length_piezo_fix, wall_thickness], center = false);
    #translate([wall_thickness + 5,  piezo_y_dist - 8.5, -5]) cylinder(d=3.5, h=100, center=false);
    #translate([wall_thickness + 35, piezo_y_dist - 8.5, -5]) cylinder(d=3.5, h=100, center=false);
  }
  difference()
  {
    translate([wall_thickness + piezo_mount_width/2, piezo_y_dist, wall_thickness - piezo_height_step]) cylinder(d=piezo_diameter, h=piezo_height_step, center=false);
    translate([wall_thickness + piezo_mount_width/2, piezo_y_dist, wall_thickness - piezo_height_step]) cylinder(d=piezo_diameter_inner, h=piezo_height_step, center=false);
    translate([0, y_length_piezo_fix, 0]) cube([piezo_mount_width, piezo_mount_width, 4*wall_thickness], center = false);
  }
}




