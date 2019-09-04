#!/bin/bash

./add_redis_command append 'size_t' 'std::string key, std::string value'

./add_redis_command dbsize 'size_t' 'size_t shard'

./add_redis_command del 'size_t' 'std::string key'

./add_redis_command del 'size_t' 'std::vector<std::string> keys'

./add_redis_command exists 'size_t' 'std::string key'

./add_redis_command exists 'size_t' 'std::vector<std::string> keys'

./add_redis_command expire 'ExpireReply' 'std::string key, std::chrono::seconds ttl'
# using ExpireReply = ::redis::ExpireReply;

#./add_redis_command get 'boost::optional<std::string>' 'std::string key'

./add_redis_command getset 'boost::optional<std::string>' 'std::string key, std::string value'

./add_redis_command hdel 'size_t' 'std::string key, std::string field'

./add_redis_command hdel 'size_t' 'std::string key, std::vector<std::string> fields'

./add_redis_command hexists 'size_t' 'std::string key, std::string field'

./add_redis_command hget 'boost::optional<std::string>' 'std::string key, std::string field'

./add_redis_command hgetall 'std::unordered_map<std::string, std::string>' 'std::string key'

./add_redis_command hincrby 'int64_t' 'std::string key, std::string field, int64_t increment'

./add_redis_command hincrbyfloat 'double' 'std::string key, std::string field, double increment'

./add_redis_command hkeys 'std::vector<std::string>' 'std::string key'
# // TODO: hscan comment

./add_redis_command hlen 'size_t' 'std::string key'

./add_redis_command hmget 'std::vector<boost::optional<std::string>>' 'std::string key, std::vector<std::string> fields'

./add_redis_command hmset 'StatusOk, void' 'std::string key, std::vector<std::pair<std::string, std::string>> field_values'

./add_redis_command hset 'HsetReply' 'std::string key, std::string field, std::string value'

./add_redis_command hsetnx 'size_t, bool' 'std::string key, std::string field, std::string value'

./add_redis_command hvals 'std::vector<std::string>' 'std::string key'
# // TODO: hscan comment

./add_redis_command incr 'int64_t' 'std::string key'

./add_redis_command keys 'std::vector<std::string>' 'std::string keys_pattern, size_t shard'

./add_redis_command lindex 'boost::optional<std::string>' 'std::string key, int64_t index'

./add_redis_command llen 'size_t' 'std::string key'

./add_redis_command lpop 'boost::optional<std::string>' 'std::string key'

./add_redis_command lpush 'size_t' 'std::string key, std::string value'

./add_redis_command lpush 'size_t' 'std::string key, std::vector<std::string> values'
# // make Llen() request if (values.empty())

./add_redis_command lrange 'std::vector<std::string>' 'std::string key, int64_t start, int64_t stop'

./add_redis_command ltrim 'StatusOk, void' 'std::string key, int64_t start, int64_t stop'

./add_redis_command mget 'std::vector<boost::optional<std::string>>' 'std::vector<std::string> keys'

./add_redis_command persist 'PersistReply' 'std::string key'
# enum class PersistReply { kTimeoutRemoved, kKeyOrTimeoutNotFound };

./add_redis_command pexpire 'ExpireReply' 'std::string key, std::chrono::milliseconds ttl'

./add_redis_command ping 'StatusPong, void' 'size_t shard'

./add_redis_command pingMessage 'std::string' 'size_t shard, std::string message'

./add_redis_command rename 'StatusOk, void' 'std::string key, std::string new_key'

./add_redis_command rpop 'boost::optional<std::string>' 'std::string key'

./add_redis_command rpush 'size_t' 'std::string key, std::string value'

./add_redis_command rpush 'size_t' 'std::string key, std::vector<std::string> values'
# // make Llen() request if (values.empty())

./add_redis_command sadd 'size_t' 'std::string key, std::string member'

./add_redis_command sadd 'size_t' 'std::string key, std::vector<std::string> members'

./add_redis_command scard 'size_t' 'std::string key'

# ./add_redis_command set 'StatusOk, void' 'std::string key, std::string value'

./add_redis_command set 'StatusOk, void' 'std::string key, std::string value, std::chrono::milliseconds ttl'

./add_redis_command setIfExist 'boost::optional<StatusOk>, bool' 'std::string key, std::string value'

./add_redis_command setIfExist 'boost::optional<StatusOk>, bool' 'std::string key, std::string value, std::chrono::milliseconds ttl'

./add_redis_command setIfNotExist 'boost::optional<StatusOk>, bool' 'std::string key, std::string value'

./add_redis_command setIfNotExist 'boost::optional<StatusOk>, bool' 'std::string key, std::string value, std::chrono::milliseconds ttl'

./add_redis_command setex 'StatusOk, void' 'std::string key, std::chrono::seconds seconds, std::string value'

./add_redis_command sismember 'size_t' 'std::string key, std::string member'

./add_redis_command smembers 'std::vector<std::string>' 'std::string key'
# // TODO: sscan comment

./add_redis_command srandmember 'boost::optional<std::string>' 'std::string key'

./add_redis_command srandmembers 'std::vector<std::string>' 'std::string key, int64_t count'
# // srandmembers -> srandmember in request

./add_redis_command srem 'size_t' 'std::string key, std::string member'

./add_redis_command srem 'size_t' 'std::string key, std::vector<std::string> members'

./add_redis_command strlen 'size_t' 'std::string key'

./add_redis_command ttl 'TtlReply' 'std::string key'

./add_redis_command type 'KeyType' 'std::string key'

./add_redis_command zadd 'size_t' 'std::string key, double score, std::string member'

./add_redis_command zadd 'size_t' 'std::string key, double score, std::string member, const ZaddOptions& options'

./add_redis_command zaddIncr 'double' 'std::string key, double score, std::string member'

./add_redis_command zaddIncrExisting 'boost::optional<double>' 'std::string key, double score, std::string member'

./add_redis_command zcard 'size_t' 'std::string key'

#./add_redis_command zrange 'std::vector<std::string>' 'std::string key, int64_t start, int64_t stop'

./add_redis_command zrangebyscore 'std::vector<std::string>' 'std::string key, double min, double max'

./add_redis_command zrangebyscore 'std::vector<std::string>' 'std::string key, std::string min, std::string max'

./add_redis_command zrangebyscore 'std::vector<std::string>' 'std::string key, double min, double max, const RangeOptions& range_options'

./add_redis_command zrangebyscore 'std::vector<std::string>' 'std::string key, std::string min, std::string max, const RangeOptions& range_options'

./add_redis_command zrangebyscoreWithScores 'std::vector<MemberScore>' 'std::string key, double min, double max'

./add_redis_command zrangebyscoreWithScores 'std::vector<MemberScore>' 'std::string key, std::string min, std::string max'

./add_redis_command zrangebyscoreWithScores 'std::vector<MemberScore>' 'std::string key, double min, double max, const RangeOptions& range_options'

./add_redis_command zrangebyscoreWithScores 'std::vector<MemberScore>' 'std::string key, std::string min, std::string max, const RangeOptions& range_options'

./add_redis_command zrem 'size_t' 'std::string key, std::string member'

./add_redis_command zrem 'size_t' 'std::string key, std::vector<std::string> members'

./add_redis_command zscore 'boost::optional<double>' 'std::string key, std::string member'
