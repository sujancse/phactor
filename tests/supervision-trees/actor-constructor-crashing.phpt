--TEST--
ONE_FOR_ONE actor crashing and restarting (from within its constructor)
--FILE--
<?php

use phactor\{ActorSystem, Actor, ActorRef, Supervisor};

$as = new ActorSystem(1);

class A extends Actor
{
    public function receive(){$this->receiveBlock();}
}

class B extends Actor
{
    private static $i = 0;

    public function __construct()
    {
        var_dump('Creating B');

        if (self::$i++ < 3) {
            var_dump('Crashing B');
            throw new exception();
        }
    }

    public function receive()
    {
        ActorSystem::shutdown();
    }
}

$a = new ActorRef(A::class, [], 'a');
$s = new Supervisor($a, Supervisor::ONE_FOR_ONE);
$b = $s->newWorker(B::class, [], 'b');
--EXPECT--
string(10) "Creating B"
string(10) "Crashing B"
string(10) "Creating B"
string(10) "Crashing B"
string(10) "Creating B"
string(10) "Crashing B"
string(10) "Creating B"
