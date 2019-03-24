

- Buffer caches should happen at the level of the HTTP client.

- Caches (at the base level) should not be async.  They shoudl be thread-safe though. Any async behaviour should be managed at a higher level with more flexibility for implementing the async / threading.

- Async should be built into the BaseCache ?