    seed_documents_query_set(dir, query_set)
    
        -> go through document collection and randomly seed it with
           terms (phrases and such) in the query set.
           
            params:
            
                how many docs, at minimum, have the term
                how many docs, at minimum, have every term in the query
                
                how many terms in the query
                    - min, max
                
                how many tokens per term
                    - min, max
                    



QuerySet Format:

{
	"label" : "label here",
	"description" : "description here",
	"corpus_path" : "path to corpus"
	"query_set" :
	[
		{ "terms" : 	
	]
}

	


this is term 1, this is term 2, term 3, term4, here's term five all of this for query 1
term1 ...for query 2
