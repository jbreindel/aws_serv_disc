#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h>
#include <aws/servicediscovery/ServiceDiscoveryClient.h>
#include <aws/servicediscovery/model/ListInstancesRequest.h>
#include <erl_nif.h>
#include <string.h>

using namespace Aws::Client;
using namespace Aws::ServiceDiscovery;
using namespace Aws::ServiceDiscovery::Model;

ERL_NIF_TERM mk_atom(ErlNifEnv* env, const char* atom) {
	ERL_NIF_TERM ret;
	if(!enif_make_existing_atom(
		env, atom, &ret, ERL_NIF_LATIN1)) {
		return enif_make_atom(env, atom);
	}
	return ret;
}

ERL_NIF_TERM mk_error(ErlNifEnv* env, const char* mesg) {
	return enif_make_tuple2(
		env, mk_atom(env, "error"), mk_atom(env, mesg));
}

static ERL_NIF_TERM list_instances(
	ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
	if (argc != 3) {
		return enif_make_badarg(env);
	}
	char serv_id[1024];
	memset(&serv_id, '\0', sizeof(serv_id));
	if (enif_get_string(env, argv[0],
		serv_id, sizeof(serv_id), ERL_NIF_LATIN1) < 1) {
		return enif_make_badarg(env);
	}
	int max_results = -1;
	if (!enif_is_atom(env, argv[1])) {
		if (!enif_is_number(env, argv[1])) {
			return enif_make_badarg(env);
		}
		if (!enif_get_int(env, argv[1], &max_results)) {
			return enif_make_badarg(env);
		}
	}
	int has_next_tok = 0;
	char next_tok[1024];
	memset(&next_tok, '\0', sizeof(next_tok));
	if (!enif_is_atom(env, argv[2])) {
		if (!enif_is_list(env, argv[2])) {
			return enif_make_badarg(env);
		}
		if (enif_get_string(env, argv[2],
			next_tok, sizeof(next_tok), ERL_NIF_LATIN1) < 1) {
			return enif_make_badarg(env);
		}
		has_next_tok = 1;
	}
	ERL_NIF_TERM res = mk_error(env, "cannot_fetch");
	Aws::SDKOptions opts;
	Aws::InitAPI(opts);
	{
		const Aws::String servId(serv_id);
		ClientConfiguration clientConfig;
		ServiceDiscoveryClient servDiscClient(clientConfig);
		ListInstancesRequest req;
		req.SetServiceId(servId);
		if (max_results > -1) {
			req.SetMaxResults(max_results);
		}
		if (has_next_tok) {
			const Aws::String nextToken(next_tok);
			req.SetNextToken(nextToken);
		}
		auto outcome = servDiscClient.ListInstances(req);
		if (outcome.IsSuccess()) {
			ListInstancesResult result = outcome.GetResult();
			Aws::Vector<InstanceSummary> summaries =
				result.GetInstances();
			ERL_NIF_TERM arr[summaries.size()];
			memset(&arr, 0, sizeof(arr));
			int err = 0;
			int i = 0;
			for (auto const &sum : summaries) {
				ERL_NIF_TERM id_key = enif_make_string(
					env, "Id", ERL_NIF_LATIN1);
				ERL_NIF_TERM id_val = enif_make_string(
					env, sum.GetId().c_str(), ERL_NIF_LATIN1);
				size_t attr_sz = sum.GetAttributes().size();
				ERL_NIF_TERM attr_keys[attr_sz];
				ERL_NIF_TERM attr_vals[attr_sz];
				int a = 0;
				for (auto const &kv : sum.GetAttributes()) {
					attr_keys[a] = enif_make_string(
						env, kv.first.c_str(), ERL_NIF_LATIN1);
					attr_vals[a] = enif_make_string(
						env, kv.second.c_str(), ERL_NIF_LATIN1);
					a++;
				}
				ERL_NIF_TERM attrs_key = enif_make_string(
					env, "Attributes", ERL_NIF_LATIN1);
				ERL_NIF_TERM attrs_val;
				if (!enif_make_map_from_arrays(
					env, attr_keys, attr_vals, attr_sz, &attrs_val)) {
					err = 1;
					break;
				}
				ERL_NIF_TERM inst;
				ERL_NIF_TERM inst_keys[2];
				ERL_NIF_TERM inst_vals[2];
				inst_keys[0] = id_key;
				inst_vals[0] = id_val;
				inst_keys[1] = attrs_key;
				inst_vals[1] = attrs_val;
				if (!enif_make_map_from_arrays(
					env, inst_keys, inst_vals, 2, &inst)) {
					err = 1;
					break;
				}
				arr[i] = inst;
				i++;
			}
			if (!err) {
				ERL_NIF_TERM insts_key = enif_make_string(
					env, "Instances", ERL_NIF_LATIN1);
				ERL_NIF_TERM insts_val = enif_make_list_from_array(
					env, arr, summaries.size());
				ERL_NIF_TERM next_token_key = enif_make_string(
					env, "NextToken", ERL_NIF_LATIN1);
				ERL_NIF_TERM next_token_val = enif_make_string(
					env, result.GetNextToken().c_str(), ERL_NIF_LATIN1);
				ERL_NIF_TERM res_map_keys[2];
				ERL_NIF_TERM res_map_vals[2];
				res_map_keys[0] = insts_key;
				res_map_vals[0] = insts_val;
				res_map_keys[1] = next_token_key;
				res_map_vals[1] = next_token_val;
				ERL_NIF_TERM res_map;
				if (enif_make_map_from_arrays(
					env, res_map_keys, res_map_vals, 2, &res_map)) {
					ERL_NIF_TERM res_atom = mk_atom(env, "ok");
					res = enif_make_tuple2(env, res_atom, res_map);
				}
			}
		} else {
			AWSError<ServiceDiscoveryErrors> err = outcome.GetError();
			ERL_NIF_TERM msg = enif_make_string(
				env, err.GetMessage().c_str(), ERL_NIF_LATIN1);
			ERL_NIF_TERM res_atom = mk_atom(env, "error");
			res = enif_make_tuple2(env, res_atom, msg);
		}
	}
	Aws::ShutdownAPI(opts);
	return res;
}

static ErlNifFunc nif_funcs[] = {
	{"list_instances", 3,
		list_instances, ERL_NIF_DIRTY_JOB_IO_BOUND}
};

ERL_NIF_INIT(aws_serv_disc,
	nif_funcs, NULL, NULL, NULL, NULL);
