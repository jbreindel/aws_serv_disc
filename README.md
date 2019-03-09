aws_serv_disc
=====

An experimental erlang nif that leverages the [aws c++ sdk](https://aws.amazon.com/sdk-for-cpp/) for service discovery.
This library provides functions `aws_serv_disc:list_instances/1,2,3`, an example is below:

```
Eshell V10.2.4  (abort with ^G)
1> aws_serv_disc:list_instances("srv-asfgdasfds21").
{ok,#{"Instances" =>
          [#{"Attributes" =>
                 #{"AVAILABILITY_ZONE" => "us-west-1c",
                   "AWS_INIT_HEALTH_STATUS" => "HEALTHY",
                   "AWS_INSTANCE_IPV4" => "10.0.2.4",
                   "AWS_INSTANCE_PORT" => "32768",
                   "EC2_INSTANCE_ID" => "i-01275c4793319ee89",
                   "ECS_CLUSTER_NAME" => "ClusterStack-Cluster",
                   "ECS_SERVICE_NAME" => "erlang-service",
                   "ECS_TASK_DEFINITION_FAMILY" => "erlang-service",
                   "REGION" => "us-west-1"},
             "Id" => "ccabd341-6b1e-4a84-99d3-a0f4c3fbbce5"}],
      "NextToken" => undefined}}
```

**This library has only been tested on ubuntu 16.04.**

Install
-----

rebar3:
```
{deps, [
	{aws_serv_disc,
		{git, "git://github.com/jbreindel/aws_serv_disc.git", {branch, "master"}}}
]}.
```

Dependencies
-----

* [erlang 21+](http://www.erlang.org/)
* [rebar3](https://www.rebar3.org/)
* [aws c++ sdk](https://aws.amazon.com/sdk-for-cpp/)

NOTE: the aws c++ sdk must be installed on the target machine.
It must be compiled as static libraries for the code to work as is:

```
$ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF ..
```

License
-----

`aws_serv_disc` is licensed under the MIT licence. See the LICENSE file for details.
