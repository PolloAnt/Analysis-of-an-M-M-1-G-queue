import queueinglib.Router;
import queueinglib.Sink;
import queueinglib.Source;
import queueinglib.Queue;


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
        queues[numberOfServers]: Queue {
            @display("p=356,160,c");
        }

        sink: Sink {
            @display("p=208,318");
        }
    connections:
        source.out --> router.in++;
        router.out++ --> sink.in++;
        for i=0..numberOfServers-1 {
            router.out++ --> queues[i].in++;
            queues[i].out --> sink.in++;
        }

}

