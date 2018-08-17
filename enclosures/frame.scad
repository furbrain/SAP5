module rod(a, b, width) {
    delta = b-a;
    length =norm(delta);
    translate(a) rotate([0,0,atan2(delta[1],delta[0])])
        translate([0,-width/2,0]) cube([length,width,width]);
}

$fn=30;

mount_holes = [[-14, 27, 0],
               [14, 27, 0]];

support_struts = [[-25, -20.43, 0],
                  [25, -20.43, 0],
                  [-25, 20.43, 0],
                  [25, 20.43, 0]];

screw_dents = [[-14.5,-38],
               [14.5,-38],
               [-14.5,-6.6],
               [14.5,-6.6]];

corners = [[-25,-38],
           [25,-38]];

module beta(width, height) {
    difference() {
        union() {
            for(i=[0,1]) {
                rod(support_struts[i], screw_dents[i+2],width);
                rod(support_struts[i], corners[i],width);
                rod(corners[i], screw_dents[i], width);
                rod(screw_dents[i+2],mount_holes[i],width);
            }
            rod(support_struts[3], mount_holes[1],width);
            rod(support_struts[3], screw_dents[3], width);
            for(i = [0,2]) {
                for(j = [mount_holes, screw_dents]) {
                    rod(j[i],j[i+1],width);
                }
            }
            for(i=mount_holes) {
                translate(i) cylinder(d=5.7+4,h=height);
            }
            for(i=support_struts) {
                translate(i) cylinder(d=6.6+4,h=height);
            }
            for(i=screw_dents) {
                translate(i) cylinder(d=6.2+4,h=6);
            }
            //laser holder
            translate([-22,-2,0]) cube([29,42,height+6]);
            
            //battery holder
            translate([-12.25,8,0]) cube([24.5,32,height]);
        }
        for(i=mount_holes) {
            translate(i) cylinder(d=5.7,h=height*3, center=true);
        }
        for(i=support_struts) {
            translate(i) cylinder(d=6.6,h=height*8, center=true);
        }
        for(i=screw_dents) {
            translate([0,0,2]) translate(i) cylinder(d=6.2,h=height+8);
        }

        //battery pack
        translate([-10.25,10,-5]) cube([20.5,28,30]);
        //laser_holder
        translate([-20,0,height+3]) cube([25,45,14]);
        translate([-17,-4,height+3]) cube([16,8,8]);
    }
}
       
//alpha();
intersection() {
    cube([55,80,100],center=true);
    beta(3,4);
}
