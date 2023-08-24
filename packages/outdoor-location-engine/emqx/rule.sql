SELECT
  nth(1, split(payload.msg, ',')) as latitude,
  nth(2, split(payload.msg, ',')) as longitude,
  nth(3, split(payload.msg, ',')) as satellites,
  nth(4, split(payload.msg, ',')) as accuracy,
  clientid
FROM
  "/location"


-- HTTP Request config
-- Type: data_to_webserver
-- path:
-- method: POST
-- headers: { "content-type": "application/x-www-form-urlencoded" }
-- body: id=${clientid}&lat=${latitude}&lon=${longitude}&accuracy=${accuracy}&sat=${satellites}
-- Resource: resource:708402