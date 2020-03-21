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

    //hole for battery connector
    translate([x_length/2, wall_thickness + piezo_mount_width + 20, wall_thickness]) #cylinder(d=8, h=z_length * 2.5, center=true);

    //hole for usb conn to wemos
    translate([18,y_length, 19 + wall_thickness]){
      translate([-1.5, 0, 0]) rotate([90,0,0]) #cylinder(d=9, h=z_length, center=true);
      translate([1.5, 0, 0]) rotate([90,0,0]) #cylinder(d=9, h=z_length, center=true);
      rotate([0,90,0]) #cube([9, z_length, 3], center = true);
    }

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

  mount_hole = 1.6;

  //led mount base
  translate([x_length/2 + piezo_mount_width/2 - led_mount_width, wall_thickness + piezo_mount_width , wall_thickness])
  {
    y_length_led_mount = y_length - piezo_mount_width - battery_y_length - wall_thickness*2;
    difference()
    {
      cube([led_mount_width, y_length_led_mount - led_cable_duct_width, led_mount_height], center = false);
      translate([led_mount_width / 2, y_length_led_mount / 2 - led_cable_duct_width / 2 + 8, 0]) #cylinder(d = 2, h = led_mount_height * 2, center = false);
      translate([led_mount_width / 2, y_length_led_mount / 2 - led_cable_duct_width / 2 - 8, 0]) #cylinder(d = 2, h = led_mount_height * 2, center = false);
    }
    
    %translate([led_mount_width / 2, y_length_led_mount / 2 - led_cable_duct_width / 2 , 15]){ 
      rotate([180,0,-90]) {include <WS2812_single_led_mount.scad>}
    }   
  }

  board_mount_height = 7;
  //wemose mount
  translate([wall_thickness + 9 + 2, wall_thickness + piezo_mount_width + 40, board_mount_height/2 + wall_thickness]) 
  {
    translate([0, 0, 0]){
      difference() {
        cube([6, 6, board_mount_height], center = true);
        #cylinder(d = mount_hole, h = 30, center = true);
      }
    }
    translate([10, -26.5, 0]){
      difference(){
        cube([6, 6, board_mount_height], center = true);
        #cylinder(d = mount_hole, h = 30, center = true);
      }
    }
    //dbg wemos board
    %translate([-9, -38.5, board_mount_height/2]) cube([26.5, 43, 3], center = false); 
  }

  //dcdc mount
  translate([6 + x_length/2 + piezo_mount_width/2, glass_mount_hole_y_dist + glass_mount_hole_diameter * 2, board_mount_height/2 + wall_thickness]) 
  {
    translate([0, 0, 0]) {
      difference(){
        cube([6, 6, board_mount_height], center = true);
        #cylinder(d = mount_hole, h = 30, center = true);
      }
    } 
    translate([7.5, 30.5, 0]) {
      difference(){
        cube([6, 6, board_mount_height], center = true);
        #cylinder(d = mount_hole, h = 30, center = true);
      }
    }
    %translate([-2.45, -2.45, board_mount_height/2]) cube([17.5, 40.5, 3], center = false);  
  }



}



