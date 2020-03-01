include<osat_cad_param.scad>


union()
{
  difference()
  {
    //main frame
    union()
    {
      difference()
      {
        cube([x_length, y_length, z_length], center = false);
        translate([wall_thickness,wall_thickness,wall_thickness])  cube([x_length - wall_thickness * 2, y_length - wall_thickness * 2 - battery_y_length, 90], center = false);
        translate([wall_thickness,wall_thickness,wall_thickness])  cube([x_length - wall_thickness * 2 - battery_x_length, y_length - wall_thickness * 2, 90], center = false);

        //battery
        translate([x_length - battery_x_length,y_length - battery_y_length,wall_thickness])  cube([x_length, y_length , 90], center = false);
      }
      //mount hole stabilizer
      translate([glass_mount_hole_x_dist, glass_mount_hole_y_dist, 0]) cylinder(d=glass_mount_hole_diameter * 2, h=z_length, center=false);
      translate([x_length - glass_mount_hole_x_dist, glass_mount_hole_y_dist, 0]) cylinder(d=glass_mount_hole_diameter * 2, h=z_length, center=false);
    }

    //mount holes
    #translate([glass_mount_hole_x_dist, glass_mount_hole_y_dist, 0]) cylinder(d=glass_mount_hole_diameter, h=z_length * 2.5, center=true);
    #translate([x_length - glass_mount_hole_x_dist, glass_mount_hole_y_dist, 0]) cylinder(d=glass_mount_hole_diameter, h=z_length * 2.5, center=true);

  }



  //Piezo mount
  translate([x_length/2 - piezo_mount_width/2, wall_thickness, wall_thickness])
  {
    difference()
    {
      cube([piezo_mount_width, piezo_mount_width, piezo_mount_height], center = false);  //piezo mount base
      translate([piezo_mount_width/2, piezo_y_dist, piezo_mount_height - piezo_height_step * 2]) cylinder(d=piezo_diameter, h=100, center=false);   //piezo mount hole
      //holes for screws
      translate([piezo_mount_width/2 - 15, piezo_y_dist + 8.5, 0]) cylinder(d=piezo_solder_fix_hole_diameter, h=100, center=false);
      translate([piezo_mount_width/2 + 15, piezo_y_dist + 8.5, 0]) cylinder(d=piezo_solder_fix_hole_diameter, h=100, center=false);

      translate([piezo_mount_width/2 - 15, piezo_y_dist - 8.5, 0]) cylinder(d=piezo_solder_fix_hole_diameter, h=100, center=false);
      translate([piezo_mount_width/2 + 15, piezo_y_dist - 8.5, 0]) cylinder(d=piezo_solder_fix_hole_diameter, h=100, center=false);
    }

    //just show other stuff in base file
    %translate([0, piezo_y_dist + 8.5 - x_lenth_solder_mount/2, 20])
    {
      include<osat_piezo_solder_fix.scad>
    }

    %translate([-wall_thickness, 0, 20])
    {
      include <osat_piezo_fix.scad>;
    }
  }
}



