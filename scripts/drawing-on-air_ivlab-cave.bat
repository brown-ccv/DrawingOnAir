@rem This will change once the program is "installed", but for now, this should start
@rem the program from the users-scratch/dfk directory


@rem Start one server for distributing events / syncing the displays
START "VRG3D Server" /D C:\V bin\vrg3d_server.exe -nogfx ivlabcave


@rem Start two instances of the program, one per graphics card
START "Drawing on Air 1 (Left and Front Walls)" /D C:\V\users-scratch\dfk\DrawingOnAir build\apps\drawtool\Release\DrawTool.exe ivlab-cave-left-and-front -f share/config/cavepainting-common.cfg -f share/config/cavepainting-cave.cfg

START "Drawing on Air 2 (Right Wall and Floor)" /D C:\V\users-scratch\dfk\DrawingOnAir build\apps\drawtool\Release\DrawTool.exe ivlab-cave-right-and-floor -f share/config/cavepainting-common.cfg -f share/config/cavepainting-cave.cfg

