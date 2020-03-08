// ****************************************************************
// 2020 IEEE SoutheastCon Hardware Competition
// Pi-Day competition
// ****************************************************************

// Conversion factors
inch  = 25.4; // mm to inch conversion
fudge = 0.1;  // mm for z-fighting

// Standard lumber dimensions
by1 = 0.75*inch;
by2 = 1.5*inch;
by4 = 3.5*inch;
halfInchPly = 15/32*inch; // nominal 0.5"

// Arena drawFloor dimensions
arenaX = 8*12*inch;
arenaY = 4*12*inch;
arenaZ = halfInchPly;

// Arena colors
floor_color  = "black";
wall_color   = "yellow";
stripe_color = "white";
button_color = "red";

stripeWidth = 1*inch;
markerWidth = 5*inch;

// Starting square and tower hole dimensions
startSquare = 12*inch;
startCenterOffset = by2 + 9.5*inch;
holeSize    = 4*inch;
holeCenterOffset = arenaX - by2 - 9.5*inch;

binWidth = 10*inch;
binDepth = 12*inch;
bin1Xoffset = arenaX/2 - (2.5*binWidth) - 3*by1;

buttonSpacing = 3*inch;



// ****************************************************************
// Draw the center line and the bin navigation marker lines
// ****************************************************************

module drawFloorLines()
{
    color(stripe_color) {

        // Stripe down the middle of the bins
        translate([startCenterOffset, (arenaY-stripeWidth)/2, arenaZ])
           cube([holeCenterOffset - startCenterOffset, stripeWidth, fudge]);

        // Draw the alignment markers for each bin on middle line
        for(bin=[0:4]) {
           translate([bin1Xoffset + 
                     (bin+1)*by1 + 
                     (bin+0.5)*binWidth -
                     stripeWidth/2,
                     (arenaY-markerWidth)/2, arenaZ])
              cube([stripeWidth, markerWidth, fudge]);
        }

        // Draw the square around the tower hole
        translate([holeCenterOffset,arenaY/2,arenaZ])
           cube([holeSize+2*stripeWidth,
                 holeSize+2*stripeWidth,
                 fudge], center=true);

    }
}


// ****************************************************************
// Draw the starting square
// ****************************************************************

module drawStartingSquare()
{
    color(stripe_color) 
        translate([startCenterOffset, arenaY/2, arenaZ])
        cube([startSquare, startSquare, fudge], center=true);
}


// ****************************************************************
// Draw the floor of the arena (4' x 8' black plywood), with a hole
//    cut out of it on end for the stacking tower (teams are 
//    responsible for providing their own stacking surface to 
//    place within the hold to isolate it from the vibrations of
//    the arena floor)
// ****************************************************************

module drawFloor()
{
    difference() {
       union() {
           // First draw the plywood floor
           color(floor_color) 
               cube([arenaX, arenaY, arenaZ]);
           drawFloorLines();
           drawStartingSquare();
       }

       // Then cut a hole out of the middle of it
       translate([holeCenterOffset,arenaY/2,-fudge])
          cube([holeSize,holeSize,(arenaZ+fudge)*3], center=true);
    }
}


// ****************************************************************
// Draw the 2x4 walls
// ****************************************************************

module drawWalls()
{
    color(wall_color) union() {

        // left wall
        translate([by2, 0, arenaZ]) 
           cube([arenaX-by2, by2, by4]);

        // right wall
        translate([0, arenaY-by2, arenaZ]) 
           cube([arenaX-by2, by2, by4]);

        // front wall (nearest tower hole)
        translate([arenaX-by2, by2, arenaZ]) 
           cube([by2, arenaY-by2, by4]);

        // back wall (pushbutton wall)
        translate([0, 0, arenaZ]) 
           cube([by2, arenaY-by2, by4]);
    }
}


// ****************************************************************
// Each bin wall is a 1x2, mounted with the 3/4" side on the plywood
//    and the 1.5" standing vertical - one wall between each bin and
//    since we have 5 bins on each wall, that is 6 walls on each side
// ****************************************************************

module drawBinWalls()
{
    color(wall_color) {

        for(wall=[0:5]) {
           translate([bin1Xoffset + wall*(binWidth+by1), 0, arenaZ]) {
              translate([0, by2, 0])
                 cube([by1,binDepth,by2]);
              translate([0, arenaY - by2 - binDepth, 0])
                 cube([by1,binDepth,by2]);
           }
        }
    }
}



// ****************************************************************
// Draw the pushbuttons
// ****************************************************************

module drawPushbuttons()
{
    for(button=[0:9]) {
        translate([by2+fudge, 
                   (button - 4.5) * buttonSpacing + arenaY/2,
                   by4/2])
        rotate([0, -90, 0])
            color(button_color)
               cylinder(r=0.5*inch, h=by2+fudge);
    }
}


// ****************************************************************
// Draw the entire arena by invoking each of the arena drawing modules
// ****************************************************************

module drawArena()
{
    drawFloor();
    drawWalls(); 
    drawBinWalls();
    drawPushbuttons();
}


// ****************************************************************
// Draw the arena, centering it over [0,0,0]
// ****************************************************************

translate([-arenaX/2,-arenaY/2, 0]) 
   drawArena();    

