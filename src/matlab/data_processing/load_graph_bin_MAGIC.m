function genes = load_graph_bin(fn_graph, num1, num2)

genes = load_regions_bin_MAGIC(fn_graph, num1, num2);
if length(genes)==0 %|| gene_cnt>100  
	% close input stream
	close_flag = -1;
	load_regions_bin(fn_graph, close_flag);
end
for j = 1:length(genes)
	genes(j).id = j;
	genes(j).initial   = double(genes(j).seg_admat(1, 2:end-1, 1)>-2);
	genes(j).terminal  = double(genes(j).seg_admat(end, 2:end-1, 1)>-2);
	genes(j).seg_admat = genes(j).seg_admat(2:end-1, 2:end-1, :);
	for s = 1:size(genes(j).segments, 2)-1
		if genes(j).segments(2, s)+1==genes(j).segments(1, s+1)
			genes(j).seg_admat(s, s+1, :) = -1;
			genes(j).seg_admat(s+1, s, :) = -1;
		end
	end

	% loop over samples and make sure that all connections 
	% can be found in each sample
	all_connections = sum(genes(j).seg_admat>-1, 3)>0; % all connections that have RNA-seq evidence in any sample
	for s = 1:size(genes(j).seg_admat, 3)
		genes(j).seg_admat(:,:,s) = max(genes(j).seg_admat(:,:,s), all_connections*2-2); 
	end

	% create pair list
	if ~isempty(genes(j).pair_mat)
		[a b] = find(sum(genes(j).pair_mat, 3)>0);
		cnt = 0;
		all_pl = zeros(length(a), 3);
		for k = 1:length(a)
			if a(k)<b(k)
				cnt = cnt+1;
				all_pl(cnt, 1:2) = [a(k), b(k)];
				for s = 1:size(genes(j).pair_mat, 3)
					all_pl(cnt, 2+s) =  genes(j).pair_mat(a(k), b(k), s);
				end
			end
		end
		all_pl(cnt+1:end, :) = [];
		genes(j).pair_list = all_pl;
	end

end

