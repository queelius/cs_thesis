<map version="1.0.1">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node CREATED="1393004629102" ID="ID_25845719" MODIFIED="1393004629102" TEXT="New Mindmap">
<node CREATED="1393004958250" ID="ID_1477477646" MODIFIED="1393004992011" POSITION="right" TEXT="information">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Formal measure of information in terms of entropy
    </p>
    <p>
      &#8226; Information = &#8220;reduction in surprise&#8221;
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1393005004761" ID="ID_67734205" MODIFIED="1393005859300" POSITION="left" TEXT="information retrieval">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      IR -- nearly everyone is familiar with this because of web search engines, like Google.
    </p>
    <p>
      Very successful, very useful, we can't do without it.
    </p>
    <p>
      
    </p>
    <p>
      IR is finding (retrieving) any kind of RELEVANT information
    </p>
    <p>
      &#160;&#160;&#160;&#160;- web pages, images, emails, documents, audio, video, applications, etc.
    </p>
    <p>
      
    </p>
    <p>
      Retrieval: find something that has been stored somewhere. In this way, we can recover information stored by you or someone else.
    </p>
    <p>
      
    </p>
    <p>
      For instance, search through emails to find an email relevant to the situation at hand.
    </p>
    <p>
      
    </p>
    <p>
      For IR to be useful, the information it retrieves must be relevant to the INFORMATION NEED of the user. The information need is usually expressed by the user as a query.
    </p>
    <p>
      
    </p>
    <p>
      What's a query? Usually, it's a list of terms, e.g., search for documents which have the following terms as members: {&quot;hello&quot;, &quot;planet&quot;, &quot;earth&quot;}. This has three terms, each a 1-gram. This is a keyword search. But a term can also consist of multiple words, e.g., {&quot;hello planet&quot;, &quot;earth&quot;}. This has two terms, one is a 2-gram and one is a 1-gram.
    </p>
    <p>
      
    </p>
    <p>
      The question is, what did the user mean by that query? That is, what should we return in response to the query, and in what order should they be returned?
    </p>
    <p>
      
    </p>
    <p>
      This is where the matching function and the ranking functions come into play.
    </p>
    <p>
      
    </p>
    <p>
      A boolean query is just where every term in the query must exist -- often exactly -- to be included in the result set.
    </p>
    <p>
      
    </p>
    <p>
      
    </p>
  </body>
</html></richcontent>
<node CREATED="1393005859259" ID="ID_537665057" MODIFIED="1393005871110" TEXT="representing meaning of documents"/>
<node CREATED="1393005875353" ID="ID_977271895" MODIFIED="1393005881942" TEXT="different models of IR">
<node CREATED="1393005883049" ID="ID_335653018" MODIFIED="1393005888198" TEXT="boolean model"/>
<node CREATED="1393005889145" ID="ID_1401527017" MODIFIED="1393005891542" TEXT="vector space model"/>
<node CREATED="1393005892585" ID="ID_1821025457" MODIFIED="1393005896308" TEXT="probablistic model"/>
<node CREATED="1393005897543" ID="ID_189752417" MODIFIED="1393005900230" TEXT="language model"/>
<node CREATED="1393005901129" ID="ID_1748012006" MODIFIED="1393005902630" TEXT="page rank"/>
</node>
<node CREATED="1393005911977" ID="ID_1847234566" MODIFIED="1393005952093" TEXT="indexing">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Facilitates rapid retrieval.
    </p>
    <p>
      
    </p>
    <p>
      How do we store the representation of the document? A bag of words?
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1393005955001" ID="ID_1069315880" MODIFIED="1393006192011" TEXT="central problem">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      We have an information seeker who has a concept of what he wants to look for.
    </p>
    <p>
      
    </p>
    <p>
      He must convert that concept into a query (usually a query consisting of terms).
    </p>
    <p>
      
    </p>
    <p>
      We have documents that express concepts and facts. We must represent these in some way, also usually as terms.
    </p>
    <p>
      
    </p>
    <p>
      Then, the central task of IR is to answer the question, how closely do the query terms match the document terms? The closer the match, the more relevant the document to the query.
    </p>
    <p>
      
    </p>
    <p>
      &quot;tragic love story&quot; -&gt; &quot;fateful star-crossed romance&quot; -- do these represent the same concepts?
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1393006236313" ID="ID_1940651834" MODIFIED="1393006379304" TEXT="relevance">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Relevance depends on the information need of the user.
    </p>
    <p>
      
    </p>
    <p>
      Timeliness?
    </p>
    <p>
      Authoritative?
    </p>
    <p>
      Topical?
    </p>
    <p>
      In general, how relevant something is is measured with respect to the information need of the user. This is subjective.
    </p>
  </body>
</html></richcontent>
<node CREATED="1393006381513" ID="ID_1370885926" MODIFIED="1393012633696" TEXT="ranking">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Both vector space and probabilistic have problems for us since they want to have a universe of terms; when we look at a query, we really only want to consider the terms in the query, not also the terms in the document. Our bloom filter representation isn't a good fit for also considering the terms in the document (we don't know what terms it has! we can only answer the question, does it have this term? with some possibility of false positive.
    </p>
    <p>
      
    </p>
    <p>
      ---
    </p>
    <p>
      A query (usually free-form, and it just consists of terms, but more complicated query models can be used, e.g., must include this keyword and not this keyword OR ...) represents the information need of the user.
    </p>
    <p>
      
    </p>
    <p>
      Rank documents by decreasing order of likelihood of relevance to the query (information need).
    </p>
    <p>
      
    </p>
    <p>
      How is ranking done, then? Many approaches.
    </p>
    <p>
      
    </p>
    <p>
      Rank by &quot;similarity&quot; (some function which takes both the document and query and outputs some value, higher values are more similar) to the query.
    </p>
    <p>
      
    </p>
  </body>
</html></richcontent>
<node CREATED="1393006704797" ID="ID_1470053877" MODIFIED="1393012816767" TEXT="heuristic">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      This approach can work well with our document (set of bloom filters representing ordered parts of the doc) representation.
    </p>
    <p>
      
    </p>
    <ul>
      <li>
        We have a rough estimate of term frequencies (granularity level)
      </li>
      <li>
        If we are trying to find not blocks but docs (wich have blocks), for a query of terms t1, t2, ..., tn, we know which does (with some probability of false positives) have the query terms. We can get inverse frequency counts and frequency counts of terms in the query.
      </li>
    </ul>
    <p>
      
    </p>
    <p>
      
    </p>
    <p>
      Some words, the &quot;key words&quot;, are repeated in a document. Give more weight to these words.
    </p>
    <p>
      
    </p>
    <p>
      Problem: biased towards long documents since they will contain more of every kind of word on average. So, normalize by document length. This gives us a value between 0 and 1.
    </p>
    <p>
      
    </p>
    <p>
      Inverse document frequency
    </p>
    <p>
      
    </p>
    <p>
      Rare words carry more meaning (have more entropy) -- the most frequent words carry almost no meaning, e.g., &quot;the&quot;. They just glue the meaningful words together. (STOP WORDS).
    </p>
    <p>
      
    </p>
    <p>
      So, change similiarity measure to account for rare words importance, e.g., sim(D,Q) = tfD(q)/|D|*log(|C|/df(q))...which is some function on the inverse of the frequency of the terms in the query, i.e., log(total number of documents / number of documents that contain q).
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1393007905961" ID="ID_152590239" MODIFIED="1393008285635" TEXT="vector space">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Documents close in vector space are closer to being about the same stuff.
    </p>
    <p>
      
    </p>
    <p>
      So, a query is also in this space, retrieve documents close to it.
    </p>
    <p>
      
    </p>
    <p>
      Measures:
    </p>
    <p>
      Dot product
    </p>
    <p>
      Cosine
    </p>
    <p>
      Dice
    </p>
    <p>
      Jaccard
    </p>
    <p>
      
    </p>
    <p>
      Rank: higher measure = higher rank = higher relevance
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1393008289145" ID="ID_473471344" MODIFIED="1393012200315" TEXT="probabilistic">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Not much different than vector space approach, but ranking is informed from probability theory (we don't know with certainty the relevance of a document to a query, so treat it probabilistically).
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1393012636265" ID="ID_1239332385" MODIFIED="1393028158246" TEXT="language model">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Treat doc as a generative model, and ask question, how likely is it that the document generated the query in question?
    </p>
    <p>
      
    </p>
    <p>
      Details: We treat each document as a language model. For instance, we could model the english language as trigrams mined from a repository. So, let's use a document model to model queries. Then we can ask, how likely is this query in this document model? P(q|d). That is, we rank documents by the probability that a query would be observed (as a sample) from the respective document model.
    </p>
    <p>
      
    </p>
    <p>
      How do we compute P(q|d)?
    </p>
    <p>
      
    </p>
    <p>
      Problems:
    </p>
    <p>
      This is not a good fit for our model; why?
    </p>
    <ul>
      <li>
        We can't store information like P(term_n | term_{n-2}, term_{n-1}). This is a trigram language model, which is pretty effective. We can only implicitly store information like P(term_n), which is just a unigram language model. Hold up. We can store ngrams, and so:

        <ul>
          <li>
            What's P(world|hello)? Can we see how often &quot;hello world&quot; ngram appears in blocks and estimate it. Not really since we don't know P(world|~hello). We don't store explicitly anything hardly, we only hash stuff into bloom filters (which represent blocks).
          </li>
        </ul>
      </li>
      <li>
        We only know (with false positives a possibility) whether a term is in a block in the doc. If the blocks were 1-term each (which would complicate other issues like ngram granularity), then it'd work. But a block can have many words in it, and we don't know if the word appears once or N times in said block. We could use counting blooms to try to estimate this, but this further complicates design and may expose too much meta-information (data confidentiality is important). How to weigh data confidentiality with this sort of stuff? Good question. In general, this doesn't seem like a good fit, although we could work with pseudo-probabilities, e.g., # blocks term is in / # blocks in doc. This means the sum of probabilities adds to |q| not 1, normalizing by |q| doesn't seem like the best thing to do though.
      </li>
    </ul>
    <p>
      
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1393028159193" ID="ID_1041624389" MODIFIED="1393028751270" TEXT="prestige - counter spamming">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Most of the time, people are looking for popular documents (or, in this case, pages from popular web sites)
    </p>
    <p>
      
    </p>
    <p>
      Use popularity of Web site (e.g., how many people visit it) to rank site pages that match given keywords.
    </p>
    <p>
      
    </p>
    <p>
      Solution: use number of hyperlinks to a site as measure of popularity (prestige) of the site.
    </p>
    <ul>
      <li>
        Count only one hyperlink from each site. Prevents link spamming.
      </li>
      <li>
        Popularity measure is for site not page.

        <ul>
          <li>
            Most hyperlinks to root of site.
          </li>
        </ul>
        <p>
          
        </p>
      </li>
    </ul>
    <p>
      PageRank: when computing prestige based on links to a site, give more weight to links from that sites have higher prestige.
    </p>
    <ul>
      <li>
        Circular definition
      </li>
      <li>
        Solve system of simultaneous equations (eigen stuff)
      </li>
      <li>
        PageRank: PR(A) = (1-d)+d * sum(i, PR(B)/C(B))
      </li>
      <li>
        Other stuff: proximity of query words &quot;information retrieval&quot; better than &quot;information...retrieval&quot;
      </li>
    </ul>
  </body>
</html></richcontent>
<node CREATED="1393028209961" ID="ID_315057961" MODIFIED="1393028290011" TEXT="page rank">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Google PageRank. Refinement of prestige.
    </p>
  </body>
</html></richcontent>
</node>
<node CREATED="1393028215977" ID="ID_480984444" MODIFIED="1393028225958" TEXT="based on popularity">
<node CREATED="1393028226569" ID="ID_442369705" MODIFIED="1393028229846" TEXT="measured by links"/>
</node>
</node>
</node>
<node CREATED="1393028754905" ID="ID_1636965130" MODIFIED="1393596229988" TEXT="keywords">
<richcontent TYPE="NOTE"><html>
  <head>
    
  </head>
  <body>
    <p>
      Keyword search
    </p>
    <p>
      
    </p>
    <p>
      Simplest sort of relevance: query string appears exactly (verbatim) in the document.
    </p>
    <p>
      
    </p>
    <p>
      Less strict: words in the query appear frequently in the document, any order (bag of words).
    </p>
    <p>
      
    </p>
    <p>
      Problems:
    </p>
    <ul>
      <li>
        May not include relevant documents that use synonymous terms
      </li>
      <li>
        May include irrelevant documents that use ambiguous terms

        <ul>
          <li>
            bit (information) vs bit (eating
          </li>
        </ul>
      </li>
    </ul>
  </body>
</html>
</richcontent>
<node CREATED="1393028941801" ID="ID_736359884" MODIFIED="1393028943445" TEXT="problems"/>
<node CREATED="1393028945449" ID="ID_1285042171" MODIFIED="1393028946805" TEXT="solutions">
<node CREATED="1393028952409" ID="ID_1717664437" MODIFIED="1393028952409" TEXT=""/>
</node>
</node>
</node>
</node>
</node>
</map>
