--TEST--
Actor class instance without implementing Actor::recieve
--DESCRIPTION--
When an Actor class instance is created, if no Actor::recieve method
is implemented, then a fatal error and a segfault is triggered.
--FILE--
<?php

$actorSystem = new ActorSystem();
class Test extends Actor {}
register('test', Test::class);
$actorSystem->block();

--EXPECTF--
Fatal error: Class Test contains 1 abstract method and must therefore be declared abstract or implement the remaining methods (Actor::receive) in %s on line %d
