         9
John  Smith is a mention, even though it has two spaces.                	     
                                                  QJ. Smith is also a mention, even though our pattern does not have the "." symbol
                	     
                                                  3J Smithjrthough is not a mention and J Smithjr is.
                	     
                                                  b
Sometimes an extraneous newline breaks a mention to John

	Smith, and we still want to match it.
                	     
                                                  ;
Other times, special characters intervene John  Smith
                	     
                                                  ;
Any maybe we even want to match no spaces:  John Smithjr.
                	     
                                         