SELECT
  payload.command as command,
  payload.status as status,
  payload.reason as reason,
  payload.method as method,
  payload.satellites as satellites,
  payload.elapsed_s as elapsed_s,
  payload.battery as battery,
  payload.uptime_s as uptime_s,
  clientid
FROM
  "/status"


-- HTTP Request config
-- Type: data_to_webserver
-- path:
-- method: POST
-- headers: { "content-type": "application/x-www-form-urlencoded" }
-- body: id=${clientid}&command=${command}&status=${status}&reason=${reason}&method=${method}&sat=${satellites}&elapsed=${elapsed_s}&batt=${battery}&uptime=${uptime_s}
-- Resource: resource:708402
