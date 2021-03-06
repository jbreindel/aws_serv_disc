-module(aws_serv_disc).
-export([
	list_instances/1,
	list_instances/2,
	list_instances/3
]).
-on_load(init/0).

-define(APPNAME, aws_serv_disc).
-define(LIBNAME, libaws_serv_disc).

list_instances(ServId) ->
	list_instances(ServId, undefined, undefined).

list_instances(ServId, MaxResults) ->
	list_instances(ServId, MaxResults, undefined).

list_instances(_, _, _) ->
	not_loaded(?LINE).

init() ->
	SoName =
		case code:priv_dir(?APPNAME) of
			{error, bad_name} ->
				case filelib:is_dir(filename:join(["..", priv])) of
					true ->
						filename:join(["..", priv, ?LIBNAME]);
					_ ->
						filename:join([priv, ?LIBNAME])
				end;
			Dir ->
				filename:join(Dir, ?LIBNAME)
		end,
	erlang:load_nif(SoName, 0).

not_loaded(Line) ->
	exit({not_loaded, [{module, ?MODULE}, {line, Line}]}).