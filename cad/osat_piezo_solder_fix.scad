include<osat_cad_param.scad>


union()
{ 
  difference()
  {
    cube([piezo_mount_width, x_lenth_solder_mount, 4], center = false);
    #translate([5,  x_lenth_solder_mount/2, -5]) cylinder(d=3.5, h=100, center=false);
    #translate([35, x_lenth_solder_mount/2, -5]) cylinder(d=3.5, h=100, center=false);
  }
}
