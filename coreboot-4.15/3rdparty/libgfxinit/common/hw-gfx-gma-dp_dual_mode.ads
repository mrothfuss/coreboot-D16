--
-- Copyright (C) 2021 Angel Pons <th3fanbus@gmail.com>
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--

with HW.GFX.DP_Dual_Mode;
pragma Elaborate_All (HW.GFX.DP_Dual_Mode);
with HW.GFX.GMA.DP_Aux_Ch;

private package HW.GFX.GMA.DP_Dual_Mode
   is new HW.GFX.DP_Dual_Mode
     (T        => DP_Port,
      Aux_Ch   => DP_Aux_Ch);
