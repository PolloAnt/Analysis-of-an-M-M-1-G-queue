import org.omnetpp.queueing.Router;
import org.omnetpp.queueing.Sink;
import org.omnetpp.queueing.Source;
import org.omnetpp.queueing.Queue;


//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
//
// TODO documentation
//
network Network
{
    parameters:
        int numberOfServers @display("bgb=593,306") = default(1);
        @display("bgb=491,414");
    submodules:
        source: Source {
            @display("p=28,32");
        }
        router: Router {
            @display("p=207,32");
        }
        queues[nunmberOfServers]: Queue {
            @display("p=333,160,c");
        }

        sink: Sink {
            @display("p=208,318");
        }
    connections:
        source.out --> router.in++;
        
        for i=0..numberOfServers-1 {
            router.out++ --> queues[i].in++;
            queues[i].out --> sink.in++;
        }
}

