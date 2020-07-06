//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include "Router.h"
#include "Job.h"
#include "Queue.h"



namespace queueing {

Define_Module(Router);

void Router::initialize()
{
    /*CUSTOM*/
    rejectedSignal = registerSignal("rejectedJobs");
    /*CUSTOM*/

    const char *algName = par("routingAlgorithm");
    if (strcmp(algName, "random") == 0) {
        routingAlgorithm = ALG_RANDOM;
    }
    else if (strcmp(algName, "roundRobin") == 0) {
        routingAlgorithm = ALG_ROUND_ROBIN;
    }
    else if (strcmp(algName, "minQueueLength") == 0) {
        routingAlgorithm = ALG_MIN_QUEUE_LENGTH;
    }
    else if (strcmp(algName, "minDelay") == 0) {
        routingAlgorithm = ALG_MIN_DELAY;
    }
    else if (strcmp(algName, "minServiceTime") == 0) {
        routingAlgorithm = ALG_MIN_SERVICE_TIME;
    }
    /*CUSTOM*/
    else if (strcmp(algName, "exactAdmissionControl") == 0) {
            routingAlgorithm = ALG_EXACT_ADMISSION_CONTROL;
        }
    /*CUSTOM*/
    rrCounter = 0;
}

void Router::handleMessage(cMessage *msg)
{
    int outGateIndex = -1;  // by default we drop the message

        switch (routingAlgorithm) {
        case ALG_RANDOM: {

            outGateIndex = par("randomGateIndex");
            break;
        }
        case ALG_ROUND_ROBIN:{

            outGateIndex = rrCounter;
            rrCounter = (rrCounter + 1) % gateSize("out");
            break;
        }
        case ALG_MIN_QUEUE_LENGTH: {
            // TODO implementation missing
            outGateIndex = -1;
            break;
        }
        case ALG_MIN_DELAY:
            // TODO implementation missing
            outGateIndex = -1;
            break;

        case ALG_MIN_SERVICE_TIME:

            // TODO implementation missing
            outGateIndex = -1;
            break;

        /*CUSTOM*/
        case ALG_EXACT_ADMISSION_CONTROL: {

            Job *job = check_and_cast<Job *>(msg);
            simtime_t jobDeadline = job->getDeadline();

            //Il numero di server nella simulazione.
            int k = getParentModule()->par("numberOfServers").intValue();

            //Vettore contenente i tempi di servizio per il nuovo job nei diversi server.
            simtime_t serviceTimes[k+1];
            serviceTimes[0] = 0; //L'indice 0 corrisponde sempre al Sink (quindi non ha un tempo di servizio associato).

            //Variabile in cui vengono sommati i tempi di servizio degli utenti in coda in un server.
            simtime_t queingJobsServiceTimes;

            //Settato a true se esiste almeno un server in grado di servire il job.
            bool serverFound = false;

            //Vettore che per ogni server indica se quel server è disponibie per servire il job.
            //Gli indici corrispondono a quelli del vettore queingJobsServiceTimes.
            bool serverAvailable[k+1];
            for(int i=0; i<=k; i++) {
                serverAvailable[i] = false;
            }

            //Iteratore che permette di scorrere tutti i gate del Router.
             for(cModule::GateIterator i(this);!i.end();i++){
                 cGate* gi = *i;

                 //Mi interessano solamente i gate di output e voglio escludere il Sink che ha sempre indice 0.
                 if(gi->getType() == cGate::OUTPUT && gi->getIndex() != 0) {
                     cObject* linkedModule = gi->getNextGate()->getOwner();

                         int gateIndex = gi->getIndex();
                         Queue *server = check_and_cast<Queue *>(linkedModule);
                         simtime_t serviceTime = server->par("serviceTime").doubleValue();

                         //Memorizzo il tempo di servizio nel vettore nella posizione corrispondente al gate di uscita.
                         serviceTimes[gateIndex] = serviceTime;

                         queingJobsServiceTimes = 0;
                         //Se ci sono dei job in coda vado a sommare i loro tempi di servizio.
                         if(server->length() != 0) {
                             //Itero sulla coda del server e sommo i tempi di servizio dei job in coda su quel server.
                             cQueue queue = server->getQueue();
                             for(cQueue::Iterator iter(queue); !iter.end(); iter++) {
                                 Job *jobInQueue = check_and_cast<Job *> (*iter);
                                 simtime_t queingJobST = jobInQueue->getServiceTime();
                                 queingJobsServiceTimes = queingJobsServiceTimes + queingJobST;

                             }

                         }

                         //Verifico se il server è in grado di servire il job.
                         if(queingJobsServiceTimes + serviceTime <= jobDeadline) {

                             //È stato trovato un server in grado di servire il job.
                             serverFound = true;
                             serverAvailable[gateIndex] = true;

                        }
                 }
             }

            //Se esiste più di un server in grado di servire il job viene scelto quello col tempo di servizio minore.
            if(serverFound) {
                simtime_t minService;

                //Viene assegnato come valore iniziale per il tempo di servizio minimo,
                //il tempo di servizio del primo server disponibile.
                for(int i=1; i<=k; i++) {
                    if(serverAvailable[i]) {
                        minService = serviceTimes[i];
                        outGateIndex = i;
                        break;
                    }
                }

                //Si cerca tra i server in grado di servire il job quello che garantisce il minor tempo di servizio.
                for(int i=1; i<=k; i++) {
                    if(serverAvailable[i] && serviceTimes[i] < minService) {
                        minService = serviceTimes[i];
                        outGateIndex = i;
                    }
                }
            }

             //Non esiste nessun server in grado di servire il job. Il job viene mandato al Sink e quindi scartato.
             else {
                 std::cout<<"Job scartato"<<endl;
                 emit(rejectedSignal, 1);
                 outGateIndex = 0;
             }

        break;
       /*CUSTOM*/
        }

        default:
            outGateIndex = -1;
            break;
        }

        // send out if the index is legal
        if (outGateIndex < 0 || outGateIndex >= gateSize("out"))
            throw cRuntimeError("Invalid output gate selected during routing");

        send(msg, "out", outGateIndex);
}

}; //namespace

